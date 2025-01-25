# IMU Driver

## Project Description

The **LSMDSO6 Driver** is a C++ library designed to simplify interaction with the LSMDSO6 sensor. This driver abstracts the complexity of the sensor's functionality, reducing the need to reference the datasheet for basic operations. It provides an intuitive interface to get your project up and running quickly.

For a quick start, refer to the **[Quick Setup](#quick-setup)** section or check the `main_i2c_test.cpp` file for a usage example.  
*Note: The provided example is not meant to be built directly, but rather to illustrate the steps required to initialize and use the driver.*


## Table of Contents
- [Project Description](#project-description)
- [Functionality](#functionality)
- [Dependencies](#dependencies)
- [Quick Setup](#quick-setup)
- [Contact](#contact)

## Functionalities
- Accelerometer (G)
- Gyroscope (dps)
- Temperature (C)

## Dependencies
- [I2C Wrapper](https://github.com/UCSOAR/CommunicationSystemsSubmodule/tree/Diogo/I2C) by Diogo Goto

## Quick Setup
1. Include the `i2c_wrapper.hpp` and `lsm6dso.hpp` 
2. Create a `DRIVER_PARAMETER` struct 
3. Enter all variables in the struct 
3. Create a `I2C_Wrapper` object with the hi2c handler created by the ioc
4. Create a `LSM6DSO_Driver` object with the `DRIVER_PARAMETER` and `I2C_Wrapper`
5. Check if `isInitialized` is TRUE<sup>1</sup>
6. Check if `maxSampFreqReached` is FALSE<sup>2, 3</sup> 
7. Create a `IMU_OUT` struct to record the data
8. Use `LSM6DSO_Driver.updStatus()` to check if data is available
9. To read data use `LSM6DSO_Driver.readSensor(&*IMU_OUT*)`
10. DONE!

1 - if `isInitialized` is FALSE use `LSM6DSO_DRIVER.updConfigs(DRIVER_PARAMETER)` method and repeat step 5 

2 - if `maxSampFreqReached` is TRUE adjust sampling frequency in the `DRIVER_PARAMETER` and call `LSM6DSO_DRIVER.updConfigs(DRIVER_PARAMETER)` and repeat step 6

3 - Even when its TRUE it's still going to work but the sampling frequency will be the 6.66KHz (The maximum sampling frequency available)
## Contact 
If you need any help or have any suggestions feel free to contact me at diogo.goto@ucalgary.ca or through Teams
