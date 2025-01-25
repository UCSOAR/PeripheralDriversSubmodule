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

void run_main(){
	imu_configs.maxG = 2;
	imu_configs.maxDPS = 2;
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
		HAL_Delay(100);
	}

}


void sendSerialUSB(IMU_OUT sensor_data) {
	char message[128];
    snprintf(message, sizeof(message), "Temp:%.2f,AccelX:%.2f,AccelY:%.2f,AccelZ:%.2f,GyroX:%.2f,GyroY:%.2f,GyroZ:%.2f\r\n",
    		sensor_data.temp,
            sensor_data.accel_x, sensor_data.accel_y, sensor_data.accel_z,
            sensor_data.gyro_x, sensor_data.gyro_y, sensor_data.gyro_z);

    HAL_UART_Transmit(&huart3, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);

}

//Function to check the configuration of the sensor in case of bit flip
