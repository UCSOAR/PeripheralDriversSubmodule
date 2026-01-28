/*
 * main_i2c_test.cpp
 *
 *  Created on: Nov 20, 2024
 *      Author: diogo
 */

#include "i2c_wrapper.hpp"
#include "lsm6dso.hpp"
#include "main_i2c_test.hpp"

DRIVER_PARAMETER imu_configs;
IMU_OUT sensor_data;

void sendSerialUSB(IMU_OUT sensor_data);
void readRegUart(I2C_Wrapper i2c_wrapper);

void run_main(){
	imu_configs.maxG = 8;
	imu_configs.maxDPS = 100;
	imu_configs.accelMinFreq = 20;
	imu_configs.gyroMinFreq = 20;
		// Create an instance of your class
		I2C_Wrapper i2c_wrapper(&hi2c1, LSM6DSO_ADDRESS);
	    LSM6DSO_Driver imu_driver(imu_configs, i2c_wrapper);




	    //Variables for the loop
	    uint8_t status = 0;

	while(1){
		if(!imu_driver.get_isInitialized()){
			imu_driver.updConfigs(imu_configs);
			continue;
		}

		status = imu_driver.updStatus();
		if(status){
			imu_driver.readSensor(&sensor_data);
			sendSerialUSB(sensor_data);
		}
		//readRegUart(i2c_wrapper);
		HAL_Delay(100);
	}

}


void sendSerialUSB(IMU_OUT sensor_data) {
	char message[128];
    snprintf(message, sizeof(message), "Temp:%.4f,AccelX:%.4f,AccelY:%.4f,AccelZ:%.4f,GyroX:%.4f,GyroY:%.4f,GyroZ:%.4f\r\n",
    		sensor_data.temp,
            sensor_data.accel_x, sensor_data.accel_y, sensor_data.accel_z,
            sensor_data.gyro_x, sensor_data.gyro_y, sensor_data.gyro_z);

    HAL_UART_Transmit(&huart3, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
}

void readRegUart(I2C_Wrapper i2c_wrapper){
	char message[128];
	i2c_wrapper.numBytes = 1;
	i2c_wrapper.registerAddress = CTRL1_XL_ADDR;
	i2c_wrapper.readReg();
	uint8_t data, sampFreq, scale;
	data = i2c_wrapper.receiveData[0];
	sampFreq = data>>4;
	scale = (data & 0x0f) >> 2;
	snprintf(message, sizeof(message),"data: %x, sampling Freq: %x, scale: %x\r\n",
			data, sampFreq, scale);

	uint8_t accel_z_hi, accel_z_lo;
	uint16_t raw_accel = 0;

	i2c_wrapper.registerAddress = 0x2C;
	i2c_wrapper.readReg();
	accel_z_lo = i2c_wrapper.receiveData[0];

	i2c_wrapper.registerAddress = 0x2D;
	i2c_wrapper.readReg();
	accel_z_hi = i2c_wrapper.receiveData[0];

	raw_accel |= accel_z_hi << 8;
	raw_accel |= accel_z_lo;

	snprintf(message, sizeof(message),"raw_accel: %d (0x%04x), high byte: 0x%02x, low byte: 0x%02x\r\n",
				raw_accel, raw_accel, accel_z_hi, accel_z_lo);
	HAL_UART_Transmit(&huart3, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
}

//Function to check the configuration of the sensor in case of bit flip
