/*
 * wheelspeed.c
 *
 *  Created on: 23 Jan 2023
 *      Author: Liam Flaherty
 */

#include "wheelspeed.h"

#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include "tasktimer/tasktimer.h"
#include "gpio/gpio.h"

REGISTERED_MODULE_STATIC_DEF(WHEELSPEED);

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime = 10 / portTICK_PERIOD_MS; // 10ms

static bool isInitialized = false;

static struct
{
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[WHEELSPEED_STACK_SIZE];

  // To simplify system timers, put this on the 100Hz event,
  // but count to 100 to do anything
  uint16_t counter;
} mTask;

static Wheelspeed_Config_T mConfig;

/*
 * Stores working data. Each byte is a single read operation.
 * Each byte contains multiple GPIO samples:
 *   uint8_t: [------ab]
 *      a: GPI for front wheel speed sensor.
 *      b: GPI for rear wheel speed sensor.
 *      -: All other bits unused.
 */
#define WSS_SAMPLES_PER_SECOND 2000U
#define WSS_SAMPLING_STREAM_SIZE_BYTES (2*WSS_SAMPLES_PER_SECOND)
#define WSS_SAMPLING_STREAM_TRIGGER_LEVEL_BYTES 1
#define WSS_BATCH_RECV_SIZE (WSS_SAMPLES_PER_SECOND)
#define WSS_MIN_SAMPLES 1930U /* Need at least this many to update the speed */
static struct
{
  uint8_t sampleStreamStorage[WSS_SAMPLING_STREAM_SIZE_BYTES];
  StaticStreamBuffer_t sampleStreamStruct;
  StreamBufferHandle_t sampleStreamHandle;
} mSampleStream;

// ------------------- Private methods -------------------
static void countLowHighTransitions(
    const uint8_t* array,
    const uint16_t len,
    VehicleState_Wheelspeed_T* out)
{
  out->wssCountFront = 0U;
  out->wssCountRear = 0U;

  // Start from index 1
  for (uint16_t i = 1; i < len; ++i) {
    /*
     * As above, the sample byte is laid out as:
     *   uint8_t: [------ab]
     *      a: GPI for front wheel speed sensor.
     *      b: GPI for rear wheel speed sensor.
     */
    uint8_t prevSample = array[i-1];
    uint8_t sample = array[i];

    // Front WSS
    if ((prevSample & 0x2) == 0 && (sample & 0x2) > 0) {
      out->wssCountFront++;
    }

    // Rear WSS
    if ((prevSample & 0x1) == 0 && (sample & 0x1) > 0) {
      out->wssCountRear++;
    }
  }
}

/**
 * @brief Calculates the RPM fields from the count field
 * 
 * @param data 
 */
static void calculateRpm(VehicleState_Wheelspeed_T* data)
{
  // This relies on the samples being collected over the period of 1s

  // Use larger int size to store 60*count

  uint32_t rpmFront = 60 * data->wssCountFront;
  rpmFront /= mConfig.sensorTeeth;

  uint32_t rpmRear = 60 * data->wssCountRear;
  rpmRear /= mConfig.sensorTeeth;

  // Truncate
  data->wheelspeedFront = (uint16_t)rpmFront;
  data->wheelspeedRear = (uint16_t)rpmRear;
}

static void Wheelspeed_TaskMethod(void)
{
  // Wait for notification to wake up
  uint32_t notifiedValue = ulTaskNotifyTake(pdTRUE, mBlockTime);
  if (notifiedValue > 0) {
    if (mTask.counter % 100U == 0U) {

      // Update sensor reading
      uint8_t samples[WSS_BATCH_RECV_SIZE];
      uint16_t nRecv = (uint16_t)xStreamBufferReceive(
          mSampleStream.sampleStreamHandle,
          samples,
          WSS_BATCH_RECV_SIZE,
          0U); // Don't block
      
      if (nRecv > 0U) {
        // count the 0->1 transitions for each bit
        VehicleState_Wheelspeed_T sensorData;
        countLowHighTransitions(samples, nRecv, &sensorData);
        calculateRpm(&sensorData);

        if (VehicleState_AccessAcquire(mConfig.state)) {
          mConfig.state->data.vehicle.wheelspeed = sensorData;
          VehicleState_AccessRelease(mConfig.state);
        }
      }

    }
    mTask.counter++;
  }
}

// LCOV_EXCL_START
static void Wheelspeed_Task(void* pvParameters)
{
  (void)pvParameters;

  while (1) {
    Wheelspeed_TaskMethod();
  }
}
// LCOV_EXCL_STOP

// ------------------- Public methods -------------------
Wheelspeed_Status_T Wheelspeed_Init(Wheelspeed_Config_T* config)
{
  DEPEND_ON(config->logging, WHEELSPEED_STATUS_ERROR_DEPENDS);
  mLog = config->logging;
  Log_Print(mLog, "Wheelspeed_Init begin\n");

  DEPEND_ON(config->state, WHEELSPEED_STATUS_ERROR_DEPENDS);
  DEPEND_ON_STATIC(TASKTIMER, WHEELSPEED_STATUS_ERROR_DEPENDS);

  mTask.counter = 0U;
  mConfig = *config;
  
  // Create serial stream for received bytes
  mSampleStream.sampleStreamHandle = xStreamBufferCreateStatic(
      WSS_SAMPLING_STREAM_SIZE_BYTES,
      WSS_SAMPLING_STREAM_TRIGGER_LEVEL_BYTES,
      mSampleStream.sampleStreamStorage,
      &mSampleStream.sampleStreamStruct);

  // Create main task
  mTask.taskHandle = xTaskCreateStatic(
      Wheelspeed_Task,
      "Wheelspeed",
      WHEELSPEED_STACK_SIZE,
      NULL,
      WHEELSPEED_TASK_PRIORITY,
      mTask.taskStack,
      &mTask.taskBuffer);

  // Register the task for timer notifications every 10ms (100Hz)
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(
      &mTask.taskHandle,
      TASKTIMER_FREQUENCY_100HZ);

  if (TASKTIMER_STATUS_OK != statusTimer) {
    return WHEELSPEED_STATUS_ERROR_INIT;
  }

  isInitialized = true;

  REGISTER_STATIC(WHEELSPEED, WHEELSPEED_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "Wheelspeed_Init complete\n");
  return WHEELSPEED_STATUS_OK;
}

BaseType_t Wheelspeed_TIM_IRQHandler(const TIM_HandleTypeDef* htim)
{
  if (!isInitialized) {
    return pdFALSE;
  }

  BaseType_t higherPriorityTaskWoken = pdFALSE;

  // Run this at like 2kHz (this would handle up to 250km/h)
  if (htim->Instance == mConfig.timerInstance) {
    /*
     * As above, the sample byte is laid out as:
     *   uint8_t: [------ab]
     *      a: GPI for front wheel speed sensor.
     *      b: GPI for rear wheel speed sensor.
     */
    uint8_t sample = 0U;

    uint8_t sampleFront = GPIO_ReadPin(mConfig.frontWsPin);
    uint8_t sampleRear = GPIO_ReadPin(mConfig.rearWsPin);

    sample |= sampleRear & 0x1;
    sample |= (uint8_t)((sampleFront & 0x1) << 1);

    xStreamBufferSendFromISR(mSampleStream.sampleStreamHandle, &sample, 1U,
                             &higherPriorityTaskWoken);
  }

  return higherPriorityTaskWoken;
}
