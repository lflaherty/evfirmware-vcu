/*
 * ad5592r.c
 *
 *  Created on: 30 Apr 2021
 *      Author: Liam Flaherty
 */

#include "ad5592r.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "comm/spi/spi.h"
#include "time/tasktimer/tasktimer.h"

#include "ad5592rRegisters.h"

// ------------------- Static data ----------------------
// Channel data definitions
#define NUM_CHANNELS 8U
typedef struct {
  AD5592R_Mode_T mode;    // DIN/DOUT/AIN/DOUT/None
  AD5592R_Pulldown_T pulldown;  // is pull-down enabled
  uint16_t value;         // Current value (be it input or output)
  // This is contained in the channelData array, and the index is the channel number
} AD5592R_ChannelData_T;

static AD5592R_ChannelData_T channelData[NUM_CHANNELS];

// Contains the "encoded" states of each of the channel modes
typedef struct {
  // encoded register values
  uint8_t enGpi;
  uint8_t enGpo;
  uint8_t enAdc;
  uint8_t enDac;
  uint8_t enPulldown;

  // counts
  uint8_t countGpi;
  uint8_t countGpo;
  uint8_t countAdc;
  uint8_t countDac;
} AD5592R_RegisterSettings_T;

static AD5592R_RegisterSettings_T dataRegisters;


// SPI Rx processing definitions
static struct {
  // Task handle for Rx task
  TaskHandle_t taskHandle;

  // Holds the TCB for the CAN Rx callback thread
  StaticTask_t xTaskBuffer;

  // Callback thread will this this as it's stack
  StackType_t xTask[AD5592R_STACK_SIZE];
} ad5592RTask;

// Tx and Rx arrays for SPI
#define SPI_MAX_LENGTH 10U   /* The longest sequence is 8 measurements + a NOP */

static SPI_Device_T spiDevice;

// Controls what the SPI bus is currently doing with the device
// TODO refactor, this doesn't make sense any more
static struct {
  bool isConfigStale; // if true, a flush of the config registers is needed
  bool isResetStale;  // if true, a reset is needed

  // mutex for synchronizing access to data
  SemaphoreHandle_t dataLock;
  StaticSemaphore_t dataLockBuffer;
} currentOperation;



// ------------------- Private methods -------------------
/*
 * Get register data for enabled channels
 */
static AD5592R_RegisterSettings_T AD5592R_GetRegisterSettings(void)
{
  AD5592R_RegisterSettings_T dataRegisters = {0};

  uint8_t channel;
  for (channel = 0; channel < NUM_CHANNELS; ++channel) {
    // shift the bit into the correct location
    switch (channelData[channel].mode) {
      case AD5592R_MODE_DOUT:
        dataRegisters.enGpo |= (1U << channel);
        dataRegisters.countGpo++;
        break;

      case AD5592R_MODE_DIN:
        dataRegisters.enGpi |= (1U << channel);
        dataRegisters.countGpi++;
        break;

      case AD5592R_MODE_AOUT:
        dataRegisters.enDac |= (1U << channel);
        dataRegisters.countDac++;
        break;

      case AD5592R_MODE_AIN:
        dataRegisters.enAdc |= (1U << channel);
        dataRegisters.countAdc++;
        break;

      default:
        // not enabled, so just leave it
        break;
    }

    if (AD5592R_PULLDOWN_ENABLED == channelData[channel].pulldown) {
      dataRegisters.enPulldown |= (0x1U << channel);
    }

  }
  return dataRegisters;
}

static AD5592R_Status_T AD5592R_SoftwareReset(void)
{
  // Encode message
  AD5592R_MessageReset_T reset;
  reset.fields.registerAddress = 0xF;
  reset.fields.reset = 0x5AC;
  reset.fields.zero = 0;

  uint16_t rxData = 0;

  if (SPI_STATUS_OK != SPI_TransmitReceiveBlocking(&spiDevice, (uint8_t*)&reset.raw, (uint8_t*)&rxData, 2)) { // 16 bits, so 2 uint8_ts
    return AD5592R_STATUS_ERROR_SPI;
  }

  // The device requires a wait of at least 250us (this waits 1ms)
  TickType_t ticks = 1 / portTICK_PERIOD_MS;
  vTaskDelay(ticks ? ticks : 1);

  return AD5592R_STATUS_OK;
}

/*
 * Writes a register
 * @param registerAddress Register address
 * @param ioValue IO0-IO7 setting
 */
static AD5592R_Status_T AD5592R_WriteRegConfig(uint8_t registerAddress, uint8_t ioValue)
{
  // encode message using union
  AD5592R_MessageConfigReg_T configMessage = {0};
  configMessage.fields.io = ioValue;
  configMessage.fields.registerAddress = registerAddress;
  configMessage.fields.zero = 0; // ensure this is always zero

  uint16_t spiRx = 0;

  if (SPI_STATUS_OK != SPI_TransmitReceiveBlocking(&spiDevice, (uint8_t*)&configMessage.raw, (uint8_t*)&spiRx, 2)) { // 16 bits, so 2 uint8_ts
    return AD5592R_STATUS_ERROR_SPI;
  }
  // No data to process, so don't need to process results (i.e. don't need to take semaphore back again)

  // The SPI callback will give the semaphore back - this isn't a normal mutex lock operation

  return AD5592R_STATUS_OK;
}

/*
 * Reads all analog inputs configured to be AIN channels and stores in channelData
 */
static AD5592R_Status_T AD5592R_AINReadAll(void)
{
  // take mutex
  if (pdTRUE != xSemaphoreTake(currentOperation.dataLock, 10U)) {
    return AD5592R_STATUS_ERROR_SEM;
  }

  uint16_t rxData = 0;
  uint16_t txData = 0;

  // Send ADC Sequence message
  AD5592R_MessageADCSequenceReg_T readAdc = {0};
  readAdc.fields.registerAddress = REG_ADC_SEQ;
  readAdc.fields.rep = 0U;  // no repetition
  readAdc.fields.temp = 0U; // don't include temperature reading TODO: actually want this
  readAdc.fields.adc = dataRegisters.enAdc;

  SPI_TransmitReceiveBlocking(&spiDevice, (uint8_t*)&readAdc.raw, (uint8_t*)&rxData, 2);

  // Send NOP
  uint16_t NOP = 0;
  rxData = 0;
  txData = NOP;
  SPI_TransmitReceiveBlocking(&spiDevice, (uint8_t*)&txData, (uint8_t*)&rxData, 2);

  // now receive all the ADC inputs
  uint8_t i;
  for (i = 0; i < dataRegisters.countAdc; ++i) {
    AD5592R_MessageADCConversionResult_T conversionResult;

    // Keep sending NOPs & decode into the ADC Conversion result union
    rxData = 0;
    txData = NOP;
    SPI_TransmitReceiveBlocking(&spiDevice, (uint8_t*)&txData, (uint8_t*)&rxData, 2);

    // copy the ADC conversion into the stored data
    conversionResult.raw = rxData;
    channelData[conversionResult.fields.adcAddress & 0x7U].value = conversionResult.fields.adcResult & 0xFFFU;
  }

  // release mutex
  xSemaphoreGive(currentOperation.dataLock);

  return AD5592R_STATUS_OK;
}

/*
 * Writes all analog outputs configured to be AOUT channels from channelData
 */
static AD5592R_Status_T AD5592R_AOUTWriteAll(void)
{
  // take mutex
  if (pdTRUE != xSemaphoreTake(currentOperation.dataLock, 10U)) {
    return AD5592R_STATUS_ERROR_SEM;
  }

  uint8_t channelNum;
  for (channelNum = 0; channelNum < NUM_CHANNELS; ++channelNum) {
    if (AD5592R_MODE_AOUT == channelData[channelNum].mode) {
      // this is a DAC channel - so write the output here
      AD5592R_MessageDACWriteReg_T writeMsg = {0};
      writeMsg.fields.dacAddress = channelNum;
      writeMsg.fields.dacData = channelData[channelNum].value;
      writeMsg.fields.one = 1U;

      // write the SPI message
      uint16_t rxData = 0;
      SPI_TransmitReceiveBlocking(&spiDevice, (uint8_t*)&writeMsg.raw, (uint8_t*)&rxData, 2);
    }
  }

  // release mutex
  xSemaphoreGive(currentOperation.dataLock);

  // TODO get an accurate return status
  return AD5592R_STATUS_OK;
}

/*
 * Reads all analog inputs configured to be DIN channels and stores in channelData
 */
static AD5592R_Status_T AD5592R_DINReadAll(void)
{
  // To do this, we write the GPIO read config register, then send a NOP
  uint16_t spiRx = 0;
  uint16_t spiTx = 0;

  // encode message using union
  AD5592R_MessageGPIOReadConfigReg_T configMessage = {0};
  configMessage.fields.gpioReadback = 0;
  configMessage.fields.zero = 0; // ensure this is always zero
  configMessage.fields.registerAddress = REG_GPIO_READ_CONFIG;
  configMessage.fields.enableReadback = 1U;

  // Determine which GPIO pins to read
  uint8_t channelNum;
  for (channelNum = 0; channelNum < NUM_CHANNELS; ++channelNum) {
    if (AD5592R_MODE_DIN == channelData[channelNum].mode) {
      configMessage.fields.gpioReadback |= (channelData[channelNum].value & 0x1) << channelNum;
    }
  }

  // Send the GPIO Read Config Register
  if (SPI_STATUS_OK != SPI_TransmitReceiveBlocking(&spiDevice, (uint8_t*)&configMessage.raw, (uint8_t*)&spiRx, 2)) { // 16 bits, so 2 uint8_ts
    return AD5592R_STATUS_ERROR_SPI;
  }

  // Send NOP
  uint16_t NOP = 0;
  spiRx = 0;
  spiTx = NOP;

  if (SPI_STATUS_OK != SPI_TransmitReceiveBlocking(&spiDevice, (uint8_t*)&spiTx, (uint8_t*)&spiRx, 2)) {
    return AD5592R_STATUS_ERROR_SPI;
  }

  // Read the GPIOs
  for (channelNum = 0; channelNum < NUM_CHANNELS; ++channelNum) {
    if (AD5592R_MODE_DIN == channelData[channelNum].mode) {
      // shift data received back down
      channelData[channelNum].value = (spiRx >> channelNum) & 0x1;
    }
  }

  return AD5592R_STATUS_OK;
}

/*
 * Writes all analog outputs configured to be DOUT channels from channelData
 */
static AD5592R_Status_T AD5592R_DOUTWriteAll(void)
{
  // take mutex
  if (pdTRUE != xSemaphoreTake(currentOperation.dataLock, 10U)) {
    return AD5592R_STATUS_ERROR_SEM;
  }

  AD5592R_MessageGPIOWriteDataReg_T writeMsg = {0};
  writeMsg.fields.zero = 0;
  writeMsg.fields.registerAddress = REG_GPIO_WRITE_DATA;
  writeMsg.fields.gpio = 0;

  // construct the GPIO control field by setting the appropriate bits
  uint8_t channelNum;
  for (channelNum = 0; channelNum < NUM_CHANNELS; ++channelNum) {
    if (AD5592R_MODE_DOUT == channelData[channelNum].mode) {
      writeMsg.fields.gpio |= (channelData[channelNum].value & 0x1) << channelNum;
    }
  }

  // write the SPI message
  uint16_t rxData = 0;
  SPI_TransmitReceiveBlocking(&spiDevice, (uint8_t*)&writeMsg.raw, (uint8_t*)&rxData, 2);

  // release mutex
  xSemaphoreGive(currentOperation.dataLock);
  return AD5592R_STATUS_OK;
}


/*
 * Initiates a register config
 */
static AD5592R_Status_T AD5592R_FlushConfig(void)
{
  // initiate an SPI write to fill the registers

  // Send messages on SPI
  AD5592R_Status_T retStatus;

  // Set up GPIs (0b1010)
  retStatus = AD5592R_WriteRegConfig(REG_GPIO_READ_CONFIG, dataRegisters.enGpi);
  if (AD5592R_STATUS_OK != retStatus) {
    return retStatus;
  }

  // Set up GPOs (0b1000)
  retStatus = AD5592R_WriteRegConfig(REG_GPIO_WRITE_CONFIG, dataRegisters.enGpo);
  if (AD5592R_STATUS_OK != retStatus) {
    return retStatus;
  }

  // Set up ADCs (0b0100)
  retStatus = AD5592R_WriteRegConfig(REG_ADC_PIN_CONFIG, dataRegisters.enAdc);
  if (AD5592R_STATUS_OK != retStatus) {
    return retStatus;
  }

  // Set up DACs (0b0101)
  retStatus = AD5592R_WriteRegConfig(REG_DAC_PIN_CONFIG, dataRegisters.enDac);
  if (AD5592R_STATUS_OK != retStatus) {
    return retStatus;
  }

  // Set up pulldowns
  retStatus = AD5592R_WriteRegConfig(REG_PULL_DOWN_CONFIG, dataRegisters.enPulldown);
  if (AD5592R_STATUS_OK != retStatus) {
    return retStatus;
  }

  return AD5592R_STATUS_OK;
}

/*
 * Task code for SPI Tx thread
 * Initiates SPI DMA tranfers
 */
static void AD5592R_PeriodicTask(void* pvParameters)
{
  printf("AD5592R_PeriodicTask begin\n");

  const TickType_t blockTime = 10 / portTICK_PERIOD_MS; // 10ms
  uint32_t notifiedValue;

  while (1) {
    // Wait for notification to wake up
    notifiedValue = ulTaskNotifyTake(pdTRUE, blockTime);
    if (notifiedValue > 0) {
      // ready to process

      // update config on device if needed
      if (pdTRUE == xSemaphoreTake(currentOperation.dataLock, 10U)) {
        if (currentOperation.isResetStale) {
          // perform reset
          AD5592R_Status_T statusReset = AD5592R_SoftwareReset();

          if (AD5592R_STATUS_OK == statusReset) {
            currentOperation.isResetStale = false;
          }
        }

        if (currentOperation.isConfigStale) {
          // flush config to device
          AD5592R_Status_T statusFlush = AD5592R_FlushConfig();

          if (AD5592R_STATUS_OK == statusFlush) {
            currentOperation.isConfigStale = false;
          } // TODO, and if statusFlush isn't OK?
        }

        // release mutex
        xSemaphoreGive(currentOperation.dataLock);
      }

      // read inputs
  //    AD5592R_AINReadAll();
  //    AD5592R_DINReadAll();

      // write outputs
      AD5592R_AOUTWriteAll();
  //    AD5592R_DOUTWriteAll();

    }
  }
}



// ------------------- Public methods -------------------
AD5592R_Status_T AD5592R_Init(
    SPI_HandleTypeDef* handle,
    GPIO_TypeDef* csPinBank,
    uint16_t csPin)
{
  printf("AD5592R_Init begin\n");

  // Initialize mem to 0
  memset(&channelData, 0, sizeof(channelData));
  memset(&currentOperation, 0, sizeof(currentOperation));
  memset(&dataRegisters, 0, sizeof(dataRegisters));

  // Setup SPI device data for the AD5592R
  // Used in the SPI_TransmitReceive method
  spiDevice.spiHandle = handle;
  spiDevice.csPinBank = csPinBank;
  spiDevice.csPin = csPin;
  spiDevice.callback = NULL;  // callback isn't used in blocking methods

  currentOperation.isConfigStale = false;
  currentOperation.isResetStale = true;

  // create data lock mutex
  currentOperation.dataLock = xSemaphoreCreateMutexStatic(&currentOperation.dataLockBuffer);
  if (NULL == currentOperation.dataLock) {
    return AD5592R_STATUS_ERROR_SEM;
  }

  // create periodic worker thread
  ad5592RTask.taskHandle = xTaskCreateStatic(
      AD5592R_PeriodicTask,
      "AD5592R_Worker",
      AD5592R_STACK_SIZE,
      NULL,
      tskIDLE_PRIORITY,
      ad5592RTask.xTask,
      &ad5592RTask.xTaskBuffer);

  // Register the task for timer notifications every 5ms
  uint16_t timerDivider = 5 / TASKTIMER_BASE_PERIOD_MS;
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&ad5592RTask.taskHandle, timerDivider);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return AD5592R_STATUS_ERROR_TIME;
  }

  printf("AD5592R_Init complete\n");
  return AD5592R_STATUS_OK;
}

//------------------------------------------------------------------------------
AD5592R_Status_T AD5592R_ConfigChannel(
    const AD5592R_Channel_T channel,
    const AD5592R_Mode_T mode,
    const AD5592R_Pulldown_T pulldown)
{
  // take mutex
  if (pdTRUE != xSemaphoreTake(currentOperation.dataLock, 10U)) {
    return AD5592R_STATUS_ERROR_SEM;
  }

  // setup config
  channelData[channel].mode = mode;
  channelData[channel].pulldown = pulldown;
  currentOperation.isConfigStale = true;

  // update register storage
  dataRegisters = AD5592R_GetRegisterSettings();

  // release mutex
  xSemaphoreGive(currentOperation.dataLock);

  // config will be flushed by task in next step

  return AD5592R_STATUS_OK;
}

//------------------------------------------------------------------------------
AD5592R_Status_T AD5592R_DINGet(const AD5592R_Channel_T channel, uint16_t* value)
{
  // check that mode is correct
  if (AD5592R_MODE_DIN != channelData[channel].mode) {
    return AD5592R_STATUS_INVALID_MODE;
  }

  // take mutex (don't return again until the mutex is released)
  if (pdTRUE != xSemaphoreTake(currentOperation.dataLock, 10U)) {
    return AD5592R_STATUS_ERROR_SEM;
  }

  // copy value from internal storage
  *value = channelData[channel].value;

  // release mutex
  xSemaphoreGive(currentOperation.dataLock);

  return AD5592R_STATUS_OK;
}

//------------------------------------------------------------------------------
AD5592R_Status_T AD5592R_DOUTSet(const AD5592R_Channel_T channel, const uint16_t value)
{
  // check data range
  if (value > 1U) {
    return AD5592R_STATUS_INVALID_VALUE;
  }

  // check that mode is correct
  if (AD5592R_MODE_DOUT != channelData[channel].mode) {
    return AD5592R_STATUS_INVALID_MODE;
  }

  // take mutex (don't return again until the mutex is released)
  if (pdTRUE != xSemaphoreTake(currentOperation.dataLock, 10U)) {
    return AD5592R_STATUS_ERROR_SEM;
  }

  channelData[channel].value = value;

  // release mutex
  xSemaphoreGive(currentOperation.dataLock);

  return AD5592R_STATUS_OK;
}

//------------------------------------------------------------------------------
AD5592R_Status_T AD5592R_AINGet(const AD5592R_Channel_T channel, uint16_t* value)
{
  // check that mode is correct
  if (AD5592R_MODE_AIN != channelData[channel].mode) {
    return AD5592R_STATUS_INVALID_MODE;
  }

  // take mutex (don't return again until the mutex is released)
  if (pdTRUE != xSemaphoreTake(currentOperation.dataLock, 10U)) {
    return AD5592R_STATUS_ERROR_SEM;
  }

  *value = channelData[channel].value;

  // release mutex
  xSemaphoreGive(currentOperation.dataLock);

  return AD5592R_STATUS_OK;
}

//------------------------------------------------------------------------------
AD5592R_Status_T AD5592R_AOUTSet(const AD5592R_Channel_T channel, const uint16_t value)
{
  // check data range
  if (value > 0xFFFU) {
    // 12 bit only
    return AD5592R_STATUS_INVALID_VALUE;
  }

  // check that mode is correct
  if (AD5592R_MODE_AOUT != channelData[channel].mode) {
    return AD5592R_STATUS_INVALID_MODE;
  }

  // take mutex (don't return again until the mutex is released)
  if (pdTRUE != xSemaphoreTake(currentOperation.dataLock, 10U)) {
    return AD5592R_STATUS_ERROR_SEM;
  }

  channelData[channel].value = value;

  // release mutex
  xSemaphoreGive(currentOperation.dataLock);

  return AD5592R_STATUS_OK;
}
