ECU Firmware
============

<h1 id="Description">Description</h1>

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

<h2 id="Dependencies">Dependencies</h2>

* Third-party:
    * FreeRTOS
    * STM32 HAL
    * Unity (C Unit Test framework)

<h1 id="Table-of-Contents">Table of Contents</h1>

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
1. [Software Components Detail](#Software-Components-Detail)
    1. [System Init Layer](#System-Init-Layer)
        1. [Init](#Init)
        1. [Global Interrupt Handler](#Global-Interrupt-Handler)
    1. [Vehicle Logic](#Vehicle-Logic)
        1. [Vehicle State Manager](#Vehicle-State-Manager)
            1. [Vehicle State](#Vehicle-State)
            1. [Fault Monitoring](#Fault-Monitoring)
                1. [LV System Faults](#LV-System-Faults)
                1. [Drive Faults](#Drive-Faults)
                1. [Special Faults](#Special-Faults)
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
    1. [Peripheral Driver/Lib Layer](#Peripheral-Driver-Lib-Layer)
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

<h1 id="Architecture">Architecture</h1>

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

<h1 id="Directory-structure">Directory structure</h1>

 * `doc` Supporting assets for docs
 * `src`
   * `cube-proj` Main entrypoint, STM32 HAL, firmware build, and STM32CubeIDE project
   * `vcu` Firmware specific to vehicle control unit.
   * `system-lib` Symlink to common MCU firmware.
 * `test` VCU tests (overlayed on top of system-lib tests)

<h1 id="Tests">Tests</h1>

The library is tested via a suite of unit tests contained under `test`. These unit tests leverge the unit testing framework, `Unity`. 

The tests can be executed by invoking `run_tests.sh`

Executing the tests will generate a code coverage report using `lcov`.

This will also invoke the unit tests from `evfirmware-lib` (`System/`)

<h1 id="Software-Components">Software Components</h1>

Expanding on the high level firmware stack from above, we can see all the software components:

<p float="left">
  <img alt="Firmware Components" src="images/Firmware_Architecture_Detailed_View.png" width="75%" />
</p>

<h2 id="User-Journeys---Internal-Operation-Examples">User Journeys - Internal Operation Examples</h2>

To visualize the flow of data through this system, we can consider a few examples:

<h3 id="Starting-vehicle---moving-into-drive-state">Starting vehicle & moving into drive state</h3>

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

<h3 id="Applying-torque-from-throttle-pedal">Applying torque from throttle pedal</h3>

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
<p float="left">
  <img alt="Detailed View - Control Example" src="images/Firmware_Architecture_Detailed_View_Example.png" width="75%" />
</p>

<h2 id="RTOS-Tasks-and-Priorities">RTOS Tasks and Priorities</h2>

The modules with RTOS tasks are arranged into the following priorities.

The RTOS (FreeRTOS) uses preemption and task priorities, and round robin scheduling for equal priorities.
Only modules that have RTOS tasks are shown here. The remaining drivers are invoked within an existing task context.

<p float="left">
  <img alt="RTOS Task Priorities" src="images/Firmware_Architecture_RTOS_task_layout.png" width="75%" />
</p>

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

<p float="left">
  <img alt="RTOS Scheduling State Machine" src="images/Firmware_Architecture_RTOS_task_state_machine.png" width="75%" />
</p>

This state machine is not explicitly coded as a state machine in the source, it is deliberate emergent behavior of the scheduler.

All critical logic registers with the watchdog handler.
The watchdog handler will trigger a system fault if any critical logic is not handled in a timely manner.

<h1 id="Software-Components-Detail">Software Components Detail</h1>

<h2 id="System-Init-Layer">System Init Layer</h2>

<h3 id="Init">Init</h3>

As the name suggests, this module is responsible for calling the init method of all other required modules in the system, and doing so in the correct order.

The init module maintains local ownership of the data structures required for all devices. It does not expose them in the global context.

The init module will create an init task, where all initialization methods are invoked from. Many init methods (for other modules) will create further RTOS tasks. Upon init completion, the init task will be deleted, however the data storage will remain.

<h3 id="Global-Interrupt-Handler">Global Interrupt Handler</h3>

This is really a sub-component of the init module, but broken out explicitly for clairty. Certain higher level drivers require some code to run from an interrupt handler. In some instances, the timing and frequency of these events would prohibit this from being done via RTOS task notifications (for example, the wheel speed sensors may run at several kHz, but perform very simple code for each ISR routine). The global interrupt handler simply implements the ISR routine, and calls each module's ISR as needed.

<h2 id="Vehicle-Logic">Vehicle Logic</h2>

<h3 id="Vehicle-State-Manager">Vehicle State Manager</h3>

The _Vehicle State Manager_ is responsible for two tasks:
* Managing the vehicle state
* Monitoring fault conditions (and handling them)

<h4 id="Vehicle-State">Vehicle State</h4>

<p float="left">
  <img alt="Vehicle State Machine" src="images/Vehicle_State_Machine.png" width="75%" />
</p>

The state diagram essentially follows:
1. Power up and wait for external devices to power up/become ready.
2. At user request, engage HV system and wait for HV to become ready.
3. Switch between driving states at user request.
4. Or enter fault state from any of the above if a fault is observed.

As noted in the diagram, there is no reverse drive state due to hill-climb/FSAE/etc vehicles having no rear visibility & no need to move in reverse.

<h4 id="Fault-Monitoring">Fault Monitoring</h4>

Faults are split into two categories: Drive Faults and LV System Faults.
* Drive faults will always invoke a transition to the fault state, regardless of origin state. 
* LV System faults will only invoke a transition to the fault state only from the `LV ready` state and onwards. Earlier during init (i.e. `LV startup` state), it is assumed that the LV systems may be in unsable states as they are initializing. After `LV startup`, all critical systems must remain in a known good state.

All faults are encoded into an error value. They are encoded as a binary one-hot system (i.e. individual faults are all a power of 2) such that multiple errors can be expressed as a logical OR of their individual error codes. Fault codes are encoded as a 32-bit word. LV System Faults occupy the lower 8 bits and Drive Faults oppupy top 24 bits.

<h5 id="LV-System-Faults">LV System Faults</h5>

| Component | Fault Condition | Configuration | Error Value | Notes |
| --------- | --------------- | ------------- | ----------- | ----- |
| Inverter/motor | CAN message timeout | Timeout period configurable | 0x00000001 | |
| BMS | CAN message timeout | Timeout period configurable | 0x00000002 | |
| Inverter | Inverter state is unexpected | | 0x00000004 | This code is only generated when the vehicle state is _LV Startup_. An unexpected inverter state from any other vehicle state would be error `0x01000000`. |
| Soft Watchdog Timeout | The soft watchdog task indicates a critical task is hung (not vehicle state manager) | | 0x00000008 | A fault in the vehicle state manager is handled as a special fault (described in [Special Faults](#Special-Faults)). |

<h5 id="Drive-Faults">Drive Faults</h5>

| Component | Fault Condition | Configuration | Error Value | Notes |
| --------- | --------------- | ------------- | ----------- | ----- |
| Accelerator Pedal | Outside of calibrated range | Calibration of pedal can be configured | 0x00000100 | A failure mode of a disconnected sensor would be handled here. |
| Accelerator Pedal | Redundant sensors disagree | Disagreement tolerance can be configured | 0x00000200 |  |
| Brake Pedal | Outside of calibrated range | Calibration of pedal can be configured | 0x00000400 | A failure mode of a disconnected sensor would be handled here. |
| Brake Pedal | Redundant sensors disagree | Disagreement tolerance can be configured | 0x00000800 |  |
| Brake Pedal | Accel/brake pedal abuse (both pedals simultaneously pressed) | Can be disabled | 0x00001000 |  |
| BMS | Any cell above temperature threshold | Limit configurable | 0x00002000 |  |
| BMS | Current draw too high | Limit configurable | 0x00004000 |  |
| BMS | Any cell above threshold voltage | Limit configurable | 0x00008000 |  |
| BMS | State of charge too low | Limit configurable | 0x00010000 |  |
| BMS | BMS fault indicator |  | 0x00020000 |  |
| Inverter/motor | Inverter internal temperature too high | Limit configurable | 0x00040000 |  |
| Inverter/motor | IGBTs above allowable temp | Limit configurable | 0x00080000 | Thermal throttling could be considered before this occurs (not currently implemented) |
| Inverter/motor | Motor above allowable temp | Limit configurable | 0x00100000 | Thermal throttling could be considered before this occurs (not currently implemented) |
| Inverter/motor | Current draw too high | Limit configurable | 0x00200000 |  |
| Inverter/motor | Inverter fault indicator |  | 0x00400000 |  |
| Inverter/motor | Inverter fault message |  | 0x00800000 |  |
| Inverter/motor | Inverter state is unexpected |  | 0x01000000 | This would result in a `0x00000004` if it occurs during the _LV Startup_ vehicle state. |

<h5 id="Special-Faults">Special Faults</h5>

A number of scenarios require more extreme management of a fault:
* A CPU fault  
_The code is configured to handle CPU faults (with a custom CPU hard fault handler)._
* A hardware watchdog timeout  
_There is a hardware watchdog in use in addition to the soft watchdog._  
_The hard watchdog will catch more extreme system hangs where the soft watchdog or RTOS scheduler has hung_
* A microcontroller brown-out
* The vehicle state manager has transitioned into a fault state, but peripherals indicate that they are still in fault state
* A soft watchdog has been triggered, but the vehicle state manager has not transitioned to it's fault state.

If these occur, the firmware will immeidately run a special fault handler where it moves the vehicle into a safe state by:
* Powering off inverter power channel
* Enabling the ECU fault output (this will open the SDC relays)
* Apply a solid LED output to indicate that the ECU is stuck
* Hold in an infite loop - we no longer proceeed past this point.

These operations are done with as little firmware layers as possible - they will write directly to hardware (i.e. they bypass the `Vehicle Control` component, and the drivers below this, as it is likely unclear what has caused this fault).

<h3 id="Soft-Watchdog">Soft Watchdog</h3>

The soft watchdog is an indepent RTOS task where, during init, other critical tasks will register to the soft watchdog. During runtime, these critical tasks must "feed" the watchdog (just call a notification task to reset their respective counters). The watchdog will count how long it has been since a feed from each critical task. If the count exceeds a timeout, it will update a flag in the _vehicle state_, where the _state machine manager_ component will read this and act accordingly.

The soft watchdog will monitor the vehicle state. If a fault state is not entered within a small amount of time, the special fault handler will be invoked.

The soft watchdog task is also responsible for feeding the hardware watchdog. The soft watchdog should be able to account for all faults that don't prevent the task from running, which is where the hardware watchdog will take over (and run the special fault handler).

<h3 id="Data-Logging">Data Logging</h3>

The data logging module simply makes a copy of system state and logs it to a file on the SDMMC at on a regular periodic interval.

<h2 id="Vehicle-Interface">Vehicle Interface</h2>
<h3 id="System-Configuration">System Configuration</h3>

TODO

<h3 id="Vehicle-Control">Vehicle Control</h3>

TODO

<h3 id="Vehicle-State">Vehicle State</h3>

TODO

<h2 id="Device-Driver-Layer">Device Driver Layer</h2>
<h3 id="Inverter">Inverter</h3>

TODO

<h3 id="BMS">BMS</h3>

TODO

<h3 id="Discrete-Sensors">Discrete Sensors</h3>

TODO

<h3 id="Wheel-Speed">Wheel Speed</h3>

TODO

<h3 id="Power-Distribution-Module-(PDM)">Power Distribution Module (PDM)</h3>

TODO

<h3 id="Shutdown-Circuit-(SDC)">Shutdown Circuit (SDC)</h3>

TODO

<h3 id="PC-Interface">PC Interface</h3>

TODO

<h3 id="Multi-purpose-IO-(MPIO)">Multi-purpose IO (MPIO)</h3>

TODO

<h3 id="IMU">IMU</h3>

TODO

<h3 id="Dashboard-output">Dashboard output</h3>

TODO

<h2 id="Peripheral-Driver-Lib-Layer">Peripheral Driver/Lib Layer</h2>
<h3 id="CAN">CAN</h3>

TODO

<h3 id="I2C">I2C</h3>

TODO

<h3 id="SPI">SPI</h3>

TODO

<h3 id="UART">UART</h3>

TODO

<h3 id="ADC">ADC</h3>

TODO

<h3 id="GPIO">GPIO</h3>

TODO

<h3 id="RTC">RTC</h3>

TODO

<h3 id="Task-Timer">Task Timer</h3>

TODO

<h3 id="CRC">CRC</h3>

TODO

<h3 id="Logging">Logging</h3>

TODO

<h3 id="Depends">Depends</h3>

TODO

<h3 id="EEPROM">EEPROM</h3>

TODO

<h2 id="STM32-HAL">STM32 HAL</h2>

The STM32 HAL is simply the ST Micro provided HAL for the STM32 F7 microcontroller.
