/*
 * orionBmsCAN.h
 *
 *  Created on: 15 Apr 2023
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_BMS_ORIONBMSCAN_H_
#define DEVICE_BMS_ORIONBMSCAN_H_

#include <stdint.h>

#define BMS_CAN_DEVICEID     0x300
#define BMS_CAN_DEVICEIDMASK 0xF00

#define BMS_CAN_ID_MAXCELLSTATE       ((uint16_t) 0x301)
#define BMS_CAN_ID_MINCELLSTATE       ((uint16_t) 0x302)
#define BMS_CAN_ID_PACKSTATE          ((uint16_t) 0x303)
#define BMS_CAN_ID_COUNTER            ((uint16_t) 0x304)


#endif /* DEVICE_BMS_ORIONBMSCAN_H_ */
