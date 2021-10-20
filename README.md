# Hillclimb EV - Vehicle Control Unit firmware

## Status

* Under development

## Description

This repository provides the firmare for Vehicle Control Unit firmware. The Vehicle Control Unit is responsible for overseeing the vehicle state and controlling the vehicle's operation from driver input.

The Vehicle Control Unit implements:
* Vehicle State Machine
* Torque controller (torque mapping to control inverter)
* Fault and safety monitoring
* Drivers for connected devices and sensors:
    * Inverter/motor controller
    * Battery Management System (BMS)
    * Analog sensors (throttle/brake pedals)
    * Wheel speed sensors
    * Dashboard

### Dependencies

* [EV System firmware: `evfirmware-lib`](https://github.com/lflaherty/evfirmware-lib) (in `System/`)
* Third-party:
    * FreeRTOS
    * STM32 HAL
    * Unity (C Unit Test framework)

## Architecture

The software is composed of two primary layers:
* Application
    Provides EV VCU specific functionality which implements the funtionality described above.
* System
    A firmware layer which abstracts the microcontroller services (such as CAN networking or analog inputs).
    This is pulled in from `evfirmware-lib` as a Git submodule.

![Components](doc/app_components.png)
![Components](System/doc/components.png)

## Tests
The library is tested via a suite of unit tests contained under `test`. These unit tests leverge the unit testing framework, `Unity`. 

The tests can be executed by invoking the Makefile under the `test` directory.

Executing the tests will generate a code coverage report using `lcov`.

This will also invoke the unit tests from `evfirmware-lib` (`System/`)