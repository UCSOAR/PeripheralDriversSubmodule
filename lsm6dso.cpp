/*
 * file: lsm6dso.cpp
 *
 *  Created on: Nov 23, 2024
 *      Author: diogo
 */


/* Includes ------------------------------------------------------------------*/
#include "lsm6dso.hpp"


	LSM6DSO_Driver::LSM6DSO_Driver(DRIVER_PARAMETER configs, I2C_Wrapper& i2c_pointer)
		:parameters(configs), i2c(i2c_pointer){

		i2c.updDeviceAddr(LSM6DSO_ADDRESS);
		i2c.numBytes = 1;
		isInitialized = true;
		maxSampFreqReached = false;

		setSensitivity();
		if(setSampFreq()){
			maxSampFreqReached = true;
		}


		i2c.sendData[0] = settings.CTRL1_XL;
		i2c.registerAddress = CTRL1_XL_ADDR;
		if(i2c.writeReg() != HAL_OK){
			isInitialized  = false;
		}

		i2c.sendData[0] = settings.CTRL2_G;
		i2c.registerAddress = CTRL2_G_ADDR;
		if(i2c.writeReg() != HAL_OK){
			isInitialized  = false;
		}
		/* Not Needed for testings
		i2c.registerAddress = CTRL4_C_ADDR;
		i2c.sendData[0] = settings.CTRL4_C;
		if(i2c.writeReg() != HAL_OK){
			isInitialized  = false;
		}

		i2c.registerAddress = CTRL5_G_ADDR;
		i2c.sendData[0] = settings.CTRL5_G;
		if(i2c.writeReg() != HAL_OK){
			isInitialized  = false;
		}

		i2c.registerAddress = CTRL6_G_ADDR;
		i2c.sendData[0] = settings.CTRL6_G;
		if(i2c.writeReg() != HAL_OK){
			isInitialized  = false;
		}

		i2c.registerAddress = CTRL7_G_ADDR;
		i2c.sendData[0] = settings.CTRL7_G;
		if(i2c.writeReg() != HAL_OK){
			isInitialized  = false;
		}

		i2c.registerAddress = CTRL10_G_ADDR;
		i2c.sendData[0] = settings.CTRL10_G;
		if(i2c.writeReg() != HAL_OK){
			isInitialized  = false;
		}
		*/

		setSensitivity();

	}

/** Member Function Implementations ------------------------------------------*/

	//Reads the STATUS_REG for new measurements
	uint8_t LSM6DSO_Driver::updStatus(void){
		i2c.updDeviceAddr(LSM6DSO_ADDRESS);
		i2c.registerAddress = STATUS_REG_ADDR;
		i2c.numBytes = 1;
		if(i2c.readReg() != HAL_OK){
			raw_data.status = 0;
			return raw_data.status;
		}
		raw_data.status = i2c.receiveData[0];
		return raw_data.status;



	}
	// Reads the measurements from the sensor and saves on dest
	uint8_t LSM6DSO_Driver::readSensor(IMU_OUT *dest){
		uint8_t status = 0;
		i2c.updDeviceAddr(LSM6DSO_ADDRESS);
		i2c.registerAddress = OUT_TEMP_L;
		i2c.numBytes = 14;
		status = i2c.readReg();
		if(status != HAL_OK){
			return status;
		}

		// Read Temperature
		raw_data.temp  = 0;
		raw_data.temp |= i2c.receiveData[0];
		raw_data.temp |= i2c.receiveData[1] << 8;

		// Reads Gyroscope
		raw_data.gyro_x  = 0;
		raw_data.gyro_x |= i2c.receiveData[2];
		raw_data.gyro_x |= i2c.receiveData[3] << 8;

		raw_data.gyro_y  = 0;
		raw_data.gyro_y |= i2c.receiveData[4];
		raw_data.gyro_y |= i2c.receiveData[5] << 8;

		raw_data.gyro_z  = 0;
		raw_data.gyro_z |= i2c.receiveData[6];
		raw_data.gyro_z |= i2c.receiveData[7] << 8;

		// Reads Accelerometer
		raw_data.accel_x  = 0;
		raw_data.accel_x |= i2c.receiveData[8];
		raw_data.accel_x |= i2c.receiveData[9] << 8;

		raw_data.accel_y  = 0;
		raw_data.accel_y |= i2c.receiveData[10];
		raw_data.accel_y |= i2c.receiveData[11] << 8;

		raw_data.accel_z  = 0;
		raw_data.accel_z |= i2c.receiveData[12];
		raw_data.accel_z |= i2c.receiveData[13] << 8;

		processData(); // applying processing on the raw data

		getReadings(dest);

		return status;

	}
	//Updates the settings
	uint8_t LSM6DSO_Driver::updConfigs(DRIVER_PARAMETER configs){
		parameters = configs;
		setSensitivity();
		maxSampFreqReached = false;
		if(setSampFreq())
			maxSampFreqReached = true;

		if(settings.CTRL1_XL >>4 <= 0x6)
			settings.CTRL6_C = 0x10;// Disable High Performance Mode if not needed (Accelerometer)
		else
			settings.CTRL6_C = 0x00;

		if(settings.CTRL1_XL >>4 <= 0x6)
			settings.CTRL7_G = 0X80; // Disable High Performance Mode if not needed (Gyroscope

		else
			settings.CTRL7_G = 0x00;


		uint8_t status = 0;
		i2c.updDeviceAddr(LSM6DSO_ADDRESS);
		i2c.numBytes = 1;

		// Configure accel power mode
		i2c.registerAddress = CTRL6_C_ADDR;
		i2c.sendData[0] = settings.CTRL6_C;
		status = i2c.writeReg();
		if(status != HAL_OK){
			return status;
		}

		// Configure gyro power mode
		i2c.registerAddress = CTRL7_G_ADDR;
		i2c.sendData[0] = settings.CTRL7_G;
		status = i2c.writeReg();
		if(status != HAL_OK){
			return status;
		}

		// Configure accel sampling frequency
		i2c.registerAddress = CTRL1_XL_ADDR;
		i2c.sendData[0] = settings.CTRL1_XL;
		status = i2c.writeReg();
		if(status != HAL_OK){
			return status;
		}
		// Configure gyro sampling frequency
		i2c.registerAddress = CTRL2_G_ADDR;
		i2c.sendData[0] = settings.CTRL2_G;
		status = i2c.writeReg();
		if(status != HAL_OK){
			return status;
		}
		/* Not needed for testings
		i2c.registerAddress = CTRL4_C_ADDR;
		i2c.sendData[0] = settings.CTRL4_C;
		status = i2c.writeReg();
		if(status != HAL_OK){
			return status;
		}

		i2c.registerAddress = CTRL5_G_ADDR;
		i2c.sendData[0] = settings.CTRL5_G;
		status = i2c.writeReg();
		if(status != HAL_OK){
			return status;
		}


		i2c.registerAddress = CTRL10_G_ADDR;
		i2c.sendData[0] = settings.CTRL10_G;
		status = i2c.writeReg();
		if(status != HAL_OK){
			return status;
		}
		*/

		setSensitivity();

		return status;
	}
	//return the last requested sensor readings
	void LSM6DSO_Driver::getReadings(IMU_OUT *dest){
		dest->temp = outputs.temp;

		dest->gyro_x = outputs.gyro_x;
		dest->gyro_y = outputs.gyro_y;
		dest->gyro_z = outputs.gyro_z;

		dest->accel_x = outputs.accel_x;
		dest->accel_y = outputs.accel_y;
		dest->accel_z = outputs.accel_z;
	}


	 bool LSM6DSO_Driver::get_isInitialized (void){
		 return isInitialized;
	 }

	 void LSM6DSO_Driver::processData(void){
		 outputs.temp = 25.0 + (raw_data.temp / 256.0);

		 outputs.accel_x = raw_data.accel_x * ACCEL_SENSITIVITY / 1000.0f;  // Convert mg to g
		 outputs.accel_y = raw_data.accel_y * ACCEL_SENSITIVITY / 1000.0f;
		 outputs.accel_z = raw_data.accel_z * ACCEL_SENSITIVITY / 1000.0f;

		 outputs.gyro_x = raw_data.gyro_x * GYRO_SENSITIVITY / 1000.0f;  // Convert mdps to dps
		 outputs.gyro_y = raw_data.gyro_y * GYRO_SENSITIVITY / 1000.0f;
		 outputs.gyro_z = raw_data.gyro_z * GYRO_SENSITIVITY / 1000.0f;
	 }

	 void LSM6DSO_Driver::setSensitivity(void){


		 if(parameters.maxG <= 2)
			 ACCEL_SENSITIVITY = LSM6DSO_ACC_SENSITIVITY_FS_2G;
		 else if(parameters.maxG <= 4)
			 ACCEL_SENSITIVITY = LSM6DSO_ACC_SENSITIVITY_FS_4G;
		 else if(parameters.maxG <= 8)
			 ACCEL_SENSITIVITY = LSM6DSO_ACC_SENSITIVITY_FS_8G;
		 else
		 	 ACCEL_SENSITIVITY = LSM6DSO_ACC_SENSITIVITY_FS_16G;


		 if(parameters.maxDPS <= 125)
			 GYRO_SENSITIVITY = LSM6DSO_GYRO_SENSITIVITY_FS_125DPS;
		 else if(parameters.maxDPS <= 250)
			 GYRO_SENSITIVITY = LSM6DSO_GYRO_SENSITIVITY_FS_250DPS;
		 else if(parameters.maxDPS <= 500)
			 GYRO_SENSITIVITY = LSM6DSO_GYRO_SENSITIVITY_FS_500DPS;
		 else if(parameters.maxDPS <= 250)
			 GYRO_SENSITIVITY = LSM6DSO_GYRO_SENSITIVITY_FS_1000DPS;
		 else
			 GYRO_SENSITIVITY = LSM6DSO_GYRO_SENSITIVITY_FS_2000DPS;

	 }
	 uint8_t LSM6DSO_Driver::setSampFreq(void){
		 uint8_t result = 0;
		 // Accelerometer Sampling Frequency
		 settings.CTRL1_XL &= 0x00;
		 if(parameters.accelMinFreq == 0)
			 settings.CTRL1_XL |= 0x00;
		 else if(parameters.accelMinFreq <= 13)
		 	settings.CTRL1_XL |= 0x10;
		 else if(parameters.accelMinFreq <= 26)
			 settings.CTRL1_XL |= 0x20;
		 else if(parameters.accelMinFreq <= 52)
			 settings.CTRL1_XL |= 0x30;
		 else if(parameters.accelMinFreq <= 104)
			 settings.CTRL1_XL |= 0x40;
		 else if(parameters.accelMinFreq <= 208)
			 settings.CTRL1_XL |= 0x50;
		 else if(parameters.accelMinFreq <= 416)
			 settings.CTRL1_XL |= 0x60;
		 else if(parameters.accelMinFreq <= 833)
			 settings.CTRL1_XL |= 0x70;
		 else if(parameters.accelMinFreq <= 1660)
			 settings.CTRL1_XL |= 0x80;
		 else if(parameters.accelMinFreq <= 3330)
			 settings.CTRL1_XL |= 0x90;
		 else if(parameters.accelMinFreq <= 6660)
			 settings.CTRL1_XL |= 0xA0;
		 else{
			 settings.CTRL1_XL |= 0xA0;
			 result = 1; // Above max sampling Frequency
		 }

		 if(parameters.maxG <= 2)
			 settings.CTRL1_XL |= 0b0000;
		 else if(parameters.maxG <= 4)
			 settings.CTRL1_XL |= 0b1000;
		 else if(parameters.maxG <= 8)
			 settings.CTRL1_XL |= 0b1100;
		 else
			 settings.CTRL1_XL |= 0b0100;

		 // Gyroscope Sampling Frequency
		 settings.CTRL2_G &= 0x0f;
		 if(parameters.gyroMinFreq == 0)
			 settings.CTRL2_G |= 0x00;
		 else if(parameters.gyroMinFreq <= 13)
		 	settings.CTRL2_G |= 0x10;
		 else if(parameters.gyroMinFreq <= 26)
			 settings.CTRL2_G |= 0x20;
		 else if(parameters.gyroMinFreq <= 52)
			 settings.CTRL2_G |= 0x30;
		 else if(parameters.gyroMinFreq <= 104)
			 settings.CTRL2_G |= 0x40;
		 else if(parameters.gyroMinFreq <= 208)
			 settings.CTRL2_G |= 0x50;
		 else if(parameters.gyroMinFreq <= 416)
			 settings.CTRL2_G |= 0x60;
		 else if(parameters.gyroMinFreq <= 833)
			 settings.CTRL2_G |= 0x70;
		 else if(parameters.gyroMinFreq <= 1660)
			 settings.CTRL2_G |= 0x80;
		 else if(parameters.gyroMinFreq <= 3330)
			 settings.CTRL2_G |= 0x90;
		 else if(parameters.gyroMinFreq <= 6660)
			 settings.CTRL2_G |= 0xA0;
		 else{
			 settings.CTRL2_G |= 0xA0;
			 result = 1; // Above max sampling Frequency
		 }

		 if(parameters.maxDPS <= 125)
			 settings.CTRL2_G |= 0b0010;
		 else if(parameters.maxDPS <= 250)
			 settings.CTRL2_G |= 0b0000;
		 else if(parameters.maxDPS <= 500)
			 settings.CTRL2_G |= 0b0100;
		 else if(parameters.maxDPS <= 1000)
			 settings.CTRL2_G |= 0b1000;
		 else
			 settings.CTRL2_G |= 0b1100;

		 return result;
	 }

	 uint8_t LSM6DSO_Driver::updCTRL1_XL(uint8_t newVal){
		 settings.CTRL1_XL = newVal;
		 i2c.updDeviceAddr(LSM6DSO_ADDRESS);
		 i2c.numBytes = 1;


		 i2c.registerAddress = CTRL1_XL_ADDR;
		 i2c.sendData[0] = settings.CTRL1_XL;
		 return i2c.writeReg();
	 }

	 uint8_t LSM6DSO_Driver::updCTRL2_G(uint8_t newVal){
		 settings.CTRL2_G = newVal;
		 i2c.updDeviceAddr(LSM6DSO_ADDRESS);
		 i2c.numBytes = 1;


		 i2c.registerAddress = CTRL2_G_ADDR;
		 i2c.sendData[0] = settings.CTRL2_G;
		 return i2c.writeReg();
	 }

	 uint8_t LSM6DSO_Driver::updCTRL6_C(uint8_t newVal){
		 settings.CTRL6_C = newVal;
		 i2c.updDeviceAddr(LSM6DSO_ADDRESS);
		 i2c.numBytes = 1;


		 i2c.registerAddress = CTRL6_C_ADDR;
		 i2c.sendData[0] = settings.CTRL6_C;
		 return i2c.writeReg();
	 }

	 uint8_t LSM6DSO_Driver::updCTRL7_G(uint8_t newVal){
		 settings.CTRL7_G = newVal;
		 i2c.updDeviceAddr(LSM6DSO_ADDRESS);
		 i2c.numBytes = 1;


		 i2c.registerAddress = CTRL7_G_ADDR;
		 i2c.sendData[0] = settings.CTRL7_G;
		 return i2c.writeReg();
	 }

