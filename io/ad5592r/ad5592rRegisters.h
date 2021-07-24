/*
 * ad5592rRegisters.h
 *
 *  Created on: Apr 30, 2021
 *      Author: Liam Flaherty
 */

#ifndef IO_AD5592R_AD5592RREGISTERS_H_
#define IO_AD5592R_AD5592RREGISTERS_H_

#include <stdint.h>

/**
 * Define the register addresses
 */

// Device registers
typedef enum {
  REG_NOP                     = 0x0U,
  REG_DAC_READBACK            = 0x1U,
  REG_ADC_SEQ                 = 0x2U,
  REG_GPIO_CONTROL            = 0x3U,
  REG_ADC_PIN_CONFIG          = 0x4U,
  REG_DAC_PIN_CONFIG          = 0x5U,
  REG_PULL_DOWN_CONFIG        = 0x6U,
  REG_READBACK_LDAC_MODE      = 0x7U,
  REG_GPIO_WRITE_CONFIG       = 0x8U,
  REG_GPIO_WRITE_DATA         = 0x9U,
  REG_GPIO_READ_CONFIG        = 0xAU,
  REG_POWER_REF_CONTROL       = 0xBU,
  REG_GPIO_OPEN_DRAIN_CONFIG  = 0xCU,
  REG_THREE_STATE_CONFIG      = 0xDU,
  // 0xE is Reserved
  REG_SOFTWARE_RESET          = 0xFU,
} AD5592R_Register_T;

/**
 * Define the bit layout of the register control messages
 */

// I/Ox Pin Configuration Registers
typedef union {
  struct {
    uint8_t io : 8; // IO0-IO7
    uint8_t reserved : 3;
    uint8_t registerAddress : 4;
    uint8_t zero : 1; // this field is always zero for the register control msg
  } fields;
  uint16_t raw;
} AD5592R_MessageConfigReg_T;

// General-Purpose Control Register
typedef union {
  struct {
    uint8_t reserved0 : 4;
    uint8_t dacRange : 1;
    uint8_t adcRange : 1;
    uint8_t allDacs : 1;
    uint8_t lock : 1;
    uint8_t adcBufferEn : 1;
    uint8_t adcBufferPrecharge : 1;
    uint8_t reserved10 : 1;
    uint8_t registerAddress : 4;
    uint8_t zero : 1; // this field is always zero for the General-Purpose Control Register
  } fields;
  uint16_t raw;
} AD5592R_MessageGeneralPurposeCtrlReg_T;

// DAC Write Register
typedef union {
  struct {
    uint16_t dacData : 12;
    uint8_t dacAddress : 3;
    uint8_t one : 1; // this field is always 1 for the DAC Write register
  } fields;
  uint16_t raw;
} AD5592R_MessageDACWriteReg_T;

// DAC Read-back Register
typedef union {
  struct {
    uint8_t dacChannel : 3;
    uint8_t enableDacReadback : 2;
    uint8_t reserved : 6;
    uint8_t registerAddress : 4; // always 0b0001
    uint8_t zero : 1; // always zero
  } fields;
  uint16_t raw;
} AD5592R_MessageDACReadback_T;

// ADC Sequence Register
typedef union {
  struct {
    uint8_t adc : 8; // ADC0-ADC7
    uint8_t temp : 1;
    uint8_t rep : 1;
    uint8_t reserved : 1;
    uint8_t registerAddress : 4; // this is always 0b0010
    uint8_t zero : 1; // this is always 0
  } fields;
  uint16_t raw;
} AD5592R_MessageADCSequenceReg_T;

// GPIO Write Configuration Register
typedef union {
  struct {
    uint8_t gpio : 8; // GPIO0-GPIO7
    uint8_t enableBusy : 1;
    uint8_t reserved : 1;
    uint8_t registerAddress : 4;
    uint8_t zero : 1; // Always zero
  } fields;
  uint16_t raw;
} AD5592R_MessageGPIOWriteConfigReg_T;

// GPIO Write Data register
typedef union {
  struct {
    uint8_t gpio : 8; // GPIO0-GPIO7
    uint8_t reserved : 3;
    uint8_t registerAddress : 4;
    uint8_t zero : 1; // always zero
  } fields;
  uint16_t raw;
} AD5592R_MessageGPIOWriteDataReg_T;

// GPIO Read Configuration Register (with enableReadback control)
typedef union {
  struct {
    uint8_t gpioReadback : 8; // GPIO0-GPIO7
    uint8_t reserved : 2;
    uint8_t enableReadback : 1;
    uint8_t registerAddress : 4; // Always 0b1010
    uint8_t zero : 1; // always 0
  } fields;
  uint16_t raw;
} AD5592R_MessageGPIOReadConfigReg_T;

// Power down mode
typedef union {
  struct {
    uint8_t powerDown : 8; // PD0-PD7
    uint8_t reserved : 1;
    uint8_t enRef : 1;
    uint8_t pdAll : 1;
    uint8_t registerAddress : 4; // this is always 0b1011
    uint8_t zero : 1; // this is always 0
  } fields;
  uint16_t raw;
} AD5592R_MessagePowerDown_T;

// Software reset
typedef union {
  struct {
    uint16_t reset : 11;  // always 0b101 1010 1100
    uint8_t registerAddress : 4; // always 0b1111
    uint8_t zero : 1; // always 0
  } fields;
  uint16_t raw;
} AD5592R_MessageReset_T;

// Read-back and LDAC Mode Register
typedef union {
  struct {
    uint8_t ldacMode : 2;
    uint8_t regReadback : 4;
    uint8_t en : 1;
    uint8_t reserved : 4;
    uint8_t registerAddress : 4; // always 0b0111
    uint8_t zero : 1; // always zero
  } fields;
  uint16_t raw;
} AD5592R_MessageReadbackLDACReg_T;

/**
 * Define the bit layout of receied messages
 */
typedef union {
  struct {
    uint16_t adcResult : 12;
    uint8_t adcAddress : 3;
    uint8_t zero : 1; // this is always 0
  } fields;
  uint16_t raw;
} AD5592R_MessageADCConversionResult_T;

#endif /* IO_AD5592R_AD5592RREGISTERS_H_ */
