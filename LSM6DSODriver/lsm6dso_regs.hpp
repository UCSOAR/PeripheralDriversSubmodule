/*
 * lsm6dso_regs.hpp
 *
 *  Created on: Nov 23, 2024
 *      Author: diogo
 */

#ifndef LSM6DSO_DRIVER_LSM6DSO_REGS_HPP_
#define LSM6DSO_DRIVER_LSM6DSO_REGS_HPP_

#define LSM6DSO_ADDRESS 0b11010100  //I2C Sensor Address

// Sensitivity Factors
//Accelerometer
#define LSM6DSO_ACC_SENSITIVITY_FS_2G   0.061f
#define LSM6DSO_ACC_SENSITIVITY_FS_4G   0.122f
#define LSM6DSO_ACC_SENSITIVITY_FS_8G   0.244f
#define LSM6DSO_ACC_SENSITIVITY_FS_16G  0.488f

// Gyroscope
#define LSM6DSO_GYRO_SENSITIVITY_FS_125DPS    4.375f
#define LSM6DSO_GYRO_SENSITIVITY_FS_250DPS    8.750f
#define LSM6DSO_GYRO_SENSITIVITY_FS_500DPS   17.500f
#define LSM6DSO_GYRO_SENSITIVITY_FS_1000DPS  35.000f
#define LSM6DSO_GYRO_SENSITIVITY_FS_2000DPS  70.000f

//Sampling Frequency
#define NORMAL_MODE_SAMPLING_FREQ 0x10 // 12.5 Hz sampling frequency in defaults mode
									   // Valid for both gyro and accel


//Controls
#define CTRL1_XL_ADDR 0x10 //Enables and select a sampling frequency for the Accelerometer
#define CTRL2_G_ADDR  0x11 //Enables and select a sampling frequency for the Gyroscope
#define CTRL4_C_ADDR  0x13 // Ultra Low Power for Accelerometer
#define CTRL5_C_ADDR  0x14 // Ultra Low Power for Gyroscope
#define CTRL6_C_ADDR  0x15 //Disables High-Perfomance mode for Accelerometer
#define CTRL7_G_ADDR  0x16 //Disables High-Perfomance mode for Gyroscope
#define CTRL10_C_ADDR 0x19 //Time stamps - 0x20 -> Enable, 0x00 -> Disable

//Outputs
#define STATUS_REG_ADDR 0x1E // New measurements available

#define OUT_TEMP_L 0x20 // Temperature [7:0]
#define OUT_TEMP_H 0x21 // Temperature [15:8]

//Gyroscope
//X-axis
#define OUTX_L_G 0x22
#define OUTX_H_G 0x23
//Y-axis
#define OUTY_L_G 0x24
#define OUTY_H_G 0x25
//Z-axis
#define OUTZ_L_G 0x26
#define OUTZ_H_G 0x27

//Accelerometer
//X-axis
#define OUTX_L_A 0x28
#define OUTX_H_A 0x29
//Y-axis
#define OUTY_L_A 0x2A
#define OUTY_H_A 0x2B
//Z-axis
#define OUTZ_L_A 0x2C
#define OUTZ_H_A 0x2D


#endif /* LSM6DSO_DRIVER_LSM6DSO_REGS_HPP_ */
