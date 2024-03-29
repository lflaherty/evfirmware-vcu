/*
 * fieldId.h
 *
 * Defines field IDs for transmitting state date
 * 
 *  Created on: Nov 20 2022
 *      Author: Liam Flaherty
 */

#ifndef VEHICLELOGIC_PCCONTROLLER_FIELDID_H_
#define VEHICLELOGIC_PCCONTROLLER_FIELDID_H_

#define PCCONTROLLER_FIELDID_SDC                    0x0001
#define PCCONTROLLER_FIELDID_PDM                    0x0002
#define PCCONTROLLER_FIELDID_BATTERY_MAXCELLVOLT    0x0003
#define PCCONTROLLER_FIELDID_BATTERY_MAXCELLVOLTID  0x0004
#define PCCONTROLLER_FIELDID_BATTERY_MAXCELLTEMP    0x0005
#define PCCONTROLLER_FIELDID_BATTERY_MAXCELLTEMPID  0x0006
#define PCCONTROLLER_FIELDID_BATTERY_DCCURRENT      0x0007
#define PCCONTROLLER_FIELDID_BATTERY_DCVOLTAGE      0x0008
#define PCCONTROLLER_FIELDID_BATTERY_SOC            0x0009
#define PCCONTROLLER_FIELDID_BATTERY_COUNTER        0x000A
#define PCCONTROLLER_FIELDID_BMS_FAULTINDICATOR     0x000B

#endif // VEHICLELOGIC_PCCONTROLLER_FIELDID_H_
