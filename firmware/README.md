ECU Firmware
============

# Description <a name="Description"/>
The ECU is responsible for overseeing the vehicle state and controlling the vehicle's operation from driver input and operating all devices associated with this process.

The ECU software implements:
* Vehicle logic:
    * Vehicle State Machine
    * Torque controller (torque mapping to control inverter)
    * Fault and safety monitoring
    * Parameter configuration
* Drivers for connected devices and sensors:
    * Inverter/motor controller
    * Battery Management System (BMS)
    * Analog sensors (throttle/brake pedals)
    * Wheel speed sensors
    * Dashboard
* Drivers for peripheral interfaces:
    * Standard interfaces: CAN, I2C, SPI, UART, ADC, GPIO, RTC, CRC
    * Task timer
    * Logging
    * init-time dependency checking
    * EEPROM

## Dependencies <a name="Dependencies"/>
* Third-party:
    * FreeRTOS
    * STM32 HAL
    * Unity (C Unit Test framework)

# Table of Contents <a name="Table-of-Contents"/>
<!-- TOC -->
1. [Description](#Description)
    1. [Dependencies](#Dependencies)
1. [Table of Contents](#Table-of-Contents)
1. [Architecture](#Architecture)
1. [Directory structure](#Directory-structure)
1. [Tests](#Tests)
1. [Software Components](#Software-Components)
    1. [User Journeys - Internal Operation Examples](#User-Journeys---Internal-Operation-Examples)
        1. [Starting vehicle & moving into drive state](#Starting-vehicle---moving-into-drive-state)
        1. [Applying torque from throttle pedal](#Applying-torque-from-throttle-pedal)
    1. [RTOS Tasks and Priorities](#RTOS-Tasks-and-Priorities)
    1. [System Init Layer](#System-Init-Layer)
        1. [Init](#Init)
        1. [Global Interrupt Handler](#Global-Interrupt-Handler)
    1. [Vehicle Logic](#Vehicle-Logic)
        1. [Vehicle State Manager](#Vehicle-State-Manager)
        1. [Soft Watchdog](#Soft-Watchdog)
        1. [Data Logging](#Data-Logging)
    1. [Vehicle Interface](#Vehicle-Interface)
        1. [System Configuration](#System-Configuration)
        1. [Vehicle Control](#Vehicle-Control)
        1. [Vehicle State](#Vehicle-State)
    1. [Device Driver Layer](#Device-Driver-Layer)
        1. [Inverter](#Inverter)
        1. [BMS](#BMS)
        1. [Discrete Sensors](#Discrete-Sensors)
        1. [Wheel Speed](#Wheel-Speed)
        1. [PC Interface](#PC-Interface)
        1. [IMU](#IMU)
        1. [Dashboard output](#Dashboard-output)
        1. [CAN](#CAN)
        1. [I2C](#I2C)
        1. [SPI](#SPI)
        1. [UART](#UART)
        1. [ADC](#ADC)
        1. [GPIO](#GPIO)
        1. [RTC](#RTC)
        1. [Task Timer](#Task-Timer)
        1. [CRC](#CRC)
        1. [Logging](#Logging)
        1. [Depends](#Depends)
        1. [EEPROM](#EEPROM)
    1. [STM32 HAL](#STM32-HAL)
<!-- END_TOC -->

# Architecture <a name="Architecture"/>
The firmware is comprised of the following layers:

1. __Vehicle Logic__  
Implements vehicle behavior including state management and vehicle controls.
2. __Vehicle Interface__  
Interfacing layer between the vehicle logic and device drivers.  
Device drivers will push data to the vehicle state, which can be pulled by vehicle logic processes. Conversely, vehicle logic processes can control the vehicle in a driver agonstic manner by using the vehicle interface layer. The vehicle interface layer will hide the driver details from the logic layer.
3. __Device Drivers__  
Implements drivers for off-board devices (e.g. inverter, wheel speed), and on-board ECU specific devices (e.g. IMU, GPS, but _not_ peripherals such as CAN or SPI). These are ECU/vehicle specific devices.
4. __Peripheral Drivers__  
Implements interfaces to common STM32 peripherals. These drivers interface to the FreeRTOS scheduling enviornment, but are otherwise agnostic to the vehicle related purpose of the ECU, and can act as generalist library. The main purpose of many of these are to provide a thread-safe manner of queueing instructions to lower level peripherals, and handling interrupts where a peripheral may be used by several _device drivers_.
5. __STM32 HAL__  
Standard HAL package provided by ST Micro.

Visualized:
<p float="left">
  <img src="images/Firmware_Architecture_Basic_View.png" width="50%" />
</p>

# Directory structure <a name="Directory-structure"/>
 * `doc` Supporting assets for docs
 * `src`
   * `cube-proj` Main entrypoint, STM32 HAL, firmware build, and STM32CubeIDE project
   * `vcu` Firmware specific to vehicle control unit.
   * `system-lib` Symlink to common MCU firmware.
 * `test` VCU tests (overlayed on top of system-lib tests)

# Tests <a name="Tests"/>
The library is tested via a suite of unit tests contained under `test`. These unit tests leverge the unit testing framework, `Unity`. 

The tests can be executed by invoking `run_tests.sh`

Executing the tests will generate a code coverage report using `lcov`.

This will also invoke the unit tests from `evfirmware-lib` (`System/`)

# Software Components <a name="Software-Components"/>
Expanding on the high level firmware stack from above, we can see all the software components:

![Firmware Components](images/Firmware_Architecture_Detailed_View.png)

## User Journeys - Internal Operation Examples <a name="User-Journeys---Internal-Operation-Examples"/>
To visualize the flow of data through this system, we can consider a few examples:

### Starting vehicle & moving into drive state <a name="Starting-vehicle---moving-into-drive-state"/>
The driver will:
1. Turn power on
2. Press brake, then simultaneously press dashboard button

The ECU firmware will, internally:

1. Power on
    1. Upon powering on, the init code will initialize all code modules.
    2. Devices will report their status to the _vehicle state_ module.
    3. The _vehicle state manager_ will query these fields until it is satisfied that the vehicle is in an idle, ready, and non fault state.
    4. When it transitions to this ready state, it instructs the _vehicle control_ module to flash in a manner that indicates this state.
    5. The _vehicle control_ module relays this requrest to the _dashboard output_.
    6. The state machine should be in the correct state now, and the driver is shown this state.
2. Driver moves vehicle into drive
    1. The driver physically presses the brake and dashboard button.
    2. At a specific polling period, the _discrete sensors_ module is recording the ADC measurements from the brake sensor ADC input, and the dashboard button input. These measurements are regularly being updated in the _vehicle state_.
    3. Simultaneously, the _vehicle state manager_ is monitoring _vehicle state_ for these fields. If the brake pressure is appropriately high, and the dashboard button has been simultaneously pressed, the _vehicle state manager_ will move through it's drive train power on process (more details in the _vehicle state manager_ doc), and if successful, will transition to the drive state. While transitioning, it instructs the _vehicle control_ module on what it needs the drive train to be doing.
    4. Once in the drive state, the _vehicle state manager_ will instruct the _vehicle control_ module to indicate on the dashboard that the car is in drive. This invokes a method in _dashboard output_ to update the indicator LED.

### Applying torque from throttle pedal <a name="Applying-torque-from-throttle-pedal"/>
Once the driver puts the vehicle into it's drive state (as above), then pressing the accelerator should apply power to the wheels.

The process internal to the ECU:

* At a regular periodic interval, the _discrete sense_ module will sample all of the ADC input sensors, including the throttle pedal sensors.
    * The sensor values are averaged. If no fault condition is found (i.e. the sensors disagree), the _discrete sense_ module will push the latest sensor value to the _vehicle state_ module.
* Simultaneously, the _vehicle control_ state will:
    1. Periodically (the task nominally runs at 100Hz/10ms) request the latest throttle sensor value. The _vehicle control_ task and sensor tasks run at the same rate. The sensors have a higher priority, so the RTOS should execute sensors first. The sensor field in _vehicle state_ is protected via a mutex.
    2. With the latest throttle sensor value, the _throttle control_ module applies a torque mapping, converting the pedal depression percentage to a requested inverter torque in Nm.
    3. The _throttle control_ module invokes the _vehicle contorl_ module to apply this value of Nm to the drive train.
    4. The _vehicle control_ will then invoke the inverter driver to send a request for this value of torque.
    5. The inverter driver constructs a CAN bus message for the requested torque output, and sends it.
* Simultaneously, the _vehicle state manager_ is monitoring the vehicle state for any fault conditions or requested state changes.

To visualize this flow of data needed by the _vehicle control_ module:
![Detailed View - Control Example](images/Firmware_Architecture_Detailed_View_Example.png)

## RTOS Tasks and Priorities <a name="RTOS-Tasks-and-Priorities"/>
The modules with RTOS tasks are arranged into the following priorities.

The RTOS (FreeRTOS) uses preemption and task priorities, and round robin scheduling for equal priorities.
Only modules that have RTOS tasks are shown here. The remaining drivers are invoked within an existing task context.

![RTOS Task Priorities](images/Firmware_Architecture_RTOS_task_layout.png)

The priorities are selected to achieve:
* Initialization is performed first, and at a higher priority than any other tasks that are started during init.  
This requires the init task to have the highest priority.
* We want the latest sensor data available for the logical tasks. This gives us the desire to have `Prio(RT critical sensors) > Prio(RT critical logic)`.
* We want real-time critical processes to have a high priority. The system performs RT critical work (i.e. driving the vehicle) alongside non-RT critical work (i.e. logging, or checking the PC interface). The RT critical work should always be performed ahead of other work, and non-RT critical work can fill the idle time in-between (most RT critical work is done at 10ms intervals, so the time after RT work has completed until the next 10ms step is available).
* The purpose of the soft watchdog is to capture a fault or hang in a critical task. All critical tasks regularly report to the soft watchdog. It is desirable to place the watchdog task as `Prio(soft watchdog) > Prio(all RT critical code)` such that:
    * Upon correct operation, the watchdog runs nominally in line with RT critical code.
    * Upon a single task having a fault where it hangs, the scheduler will context switch to the soft watchdog, whose internal counter will identify a fault.
    * If the fault causes the entire system (including scheduler) to lock up, the hardware watchdog will take over.
* Non RT critical tasks can be arranged on lower priorities.
    * Non RT critical sensors (useful for log data) are put on a higher priority than the log task for the same reason as above with the RT critical sensors/logic.

These priorities, in conjunction with the scheduler, enact the following de facto state machine:

![RTOS Scheduling State Machine](images/Firmware_Architecture_RTOS_task_state_machine.png)

This state machine is not explicitly coded as a state machine in the source, it is deliberate emergent behavior of the scheduler.

All critical logic registers with the watchdog handler.
The watchdog handler will trigger a system fault if any critical logic is not handled in a timely manner.


## System Init Layer <a name="System-Init-Layer"/>
### Init <a name="Init"/>

As the name suggests, this module is responsible for calling the init method of all other required modules in the system, and doing so in the correct order.

The init module maintains local ownership of the data structures required for all devices. It does not expose them in the global context.

The init module will create an init task, where all initialization methods are invoked from. Many init methods (for other modules) will create further RTOS tasks. Upon init completion, the init task will be deleted, however the data storage will remain.

### Global Interrupt Handler <a name="Global-Interrupt-Handler"/>

This is really a sub-component of the init module, but broken out explicitly for clairty. Certain higher level drivers require some code to run from an interrupt handler. In some instances, the timing and frequency of these events would prohibit this from being done via RTOS task notifications (for example, the wheel speed sensors may run at several kHz, but perform very simple code for each ISR routine). The global interrupt handler simply implements the ISR routine, and calls each module's ISR as needed.

## Vehicle Logic <a name="Vehicle-Logic"/>
TODO

### Vehicle State Manager <a name="Vehicle-State-Manager"/>
TODO

### Soft Watchdog <a name="Soft-Watchdog"/>
TODO

### Data Logging <a name="Data-Logging"/>
TODO

## Vehicle Interface <a name="Vehicle-Interface"/>
### System Configuration <a name="System-Configuration"/>
TODO

### Vehicle Control <a name="Vehicle-Control"/>
TODO

### Vehicle State <a name="Vehicle-State"/>
TODO

## Device Driver Layer <a name="Device-Driver-Layer"/>
### Inverter <a name="Inverter"/>
TODO

### BMS <a name="BMS"/>
TODO

### Discrete Sensors <a name="Discrete-Sensors"/>
TODO

### Wheel Speed <a name="Wheel-Speed"/>
TODO

### Power Distribution Module (PDM) <a name="Power-Distribution-Module-(PDM)"/>
TODO

### Shutdown Circuit (SDC) <a name="Shutdown-Circuit-(SDC)"/>
TODO

### PC Interface <a name="PC-Interface"/>
TODO

### Multi-purpose IO (MPIO) <a name="Multi-purpose-IO-(MPIO)"/>
TODO

### IMU <a name="IMU"/>
TODO

### Dashboard output <a name="Dashboard-output"/>
TODO

## Peripheral Driver/Lib Layer <a name="Peripheral-Driver-Lib-Layer"/>
### CAN <a name="CAN"/>
TODO

### I2C <a name="I2C"/>
TODO

### SPI <a name="SPI"/>
TODO

### UART <a name="UART"/>
TODO

### ADC <a name="ADC"/>
TODO

### GPIO <a name="GPIO"/>
TODO

### RTC <a name="RTC"/>
TODO

### Task Timer <a name="Task-Timer"/>
TODO

### CRC <a name="CRC"/>
TODO

### Logging <a name="Logging"/>
TODO

### Depends <a name="Depends"/>
TODO

### EEPROM <a name="EEPROM"/>
TODO

## STM32 HAL <a name="STM32-HAL"/>
The STM32 HAL is simply the ST Micro provided HAL for the STM32 F7 microcontroller.
