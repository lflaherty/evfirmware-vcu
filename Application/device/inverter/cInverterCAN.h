/*
 * cInverterCAN_IDs.h
 *
 *  Created on: Oct 7 2021
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_INVERTER_CINVERTERCAN_H_
#define DEVICE_INVERTER_CINVERTERCAN_H_

#define CINVERTER_CAN_TEMPERATURES1       ((uint16_t) 0x0A0U)
#define CINVERTER_CAN_TEMPERATURES2       ((uint16_t) 0x0A1U)
#define CINVERTER_CAN_TEMPERATURES3       ((uint16_t) 0x0A2U)
#define CINVERTER_CAN_MOTOR_POS_INFO      ((uint16_t) 0x0A5U)
#define CINVERTER_CAN_CURRENT_INFO        ((uint16_t) 0x0A6U)
#define CINVERTER_CAN_VOLTAGE_INFO        ((uint16_t) 0x0A7U)
#define CINVERTER_CAN_FLUX_INFO           ((uint16_t) 0x0A8U)
#define CINVERTER_CAN_INTERNAL_STATES     ((uint16_t) 0x0A9U)
#define CINVERTER_CAN_FAULT_CODES         ((uint16_t) 0x0ABU)
#define CINVERTER_CAN_TORQUE_TIMER        ((uint16_t) 0x0ACU)
#define CINVERTER_CAN_FLUX_WEAKENING      ((uint16_t) 0x0ADU)


#endif /* DEVICE_INVERTER_CINVERTERCAN_H_ */
