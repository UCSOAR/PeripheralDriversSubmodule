/*
 * lsm6dso.hpp
 *
 *  Created on: Nov 23, 2024
 *      Author: diogo
 */

#ifndef LSM6DSO_DRIVER_LSM6DSO_HPP_
#define LSM6DSO_DRIVER_LSM6DSO_HPP_

#include "lsm6dso_regs.hpp" // Defines the address of some of the registers
#include "i2c_wrapper.hpp"

typedef struct LSM6DSO_CONFIGS{
		uint8_t CTRL1_XL; // Use normal mode from the lsm6dso_regs.hpp
		uint8_t CTRL2_G;  // ||
		/*
*		uint8_t CTRL4_C;  // Ultra Low Power for Accelerometer
*		uint8_t CTRL5_C;  // Ultra Low Power for Gyroscope
*		uint8_t CTRL6_C;  // High Performance Mode for Accelerometer
*		uint8_t CTRL7_G;  // High Performance Mode for Gyroscope
*		uint8_t CTRL8_XL; // Filtering for both sensors
*		uint8_t CTRL10_C; // Timestamp 0x20 -> Enable, 0x00 -> Disable

*		uint8_t INT1_CTRL;
*		uint8_t WAKE_UP_DUR;
*		uint8_t FREE_FALL;
*		uint8_t MD1_CFG;
*		uint8_t FIFO_CTRL4;
 */

// * = not_needed for testings prototypes

} IMU_CONFIGS;

typedef struct DRIVER_PARAMETER{
	uint8_t maxG;
	uint8_t maxDPS;
	uint16_t accelMinFreq;
	uint16_t gyroMinFreq;
}PARAMETERS;

typedef struct LSM6DSO_RAW{
	uint8_t status;

	int16_t temp;

	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;

	int16_t accel_x;
	int16_t accel_y;
	int16_t accel_z;
}IMU_RAW;

typedef struct LSM6DSO_OUTPUT{
	float temp; // in Celcius

	float gyro_x; // in G
	float gyro_y; // in G
	float gyro_z; // in G

	float accel_x; //in dps
	float accel_y; //in dps
	float accel_z; //in dps
} IMU_OUT;

class LSM6DSO_Driver{
public:
	//Constructor
	//=================================================================
	/**
	* @brief Constructs the LSM6_DSO Driver.
	* @param configs .
	* @param deviceAddress 8-bit device address.
	*/
	LSM6DSO_Driver(DRIVER_PARAMETER configs, I2C_Wrapper& i2c_pointer);
	//=================================================================

	//Function Definitions
	//=================================================================
	uint8_t updStatus(void);   //Reads the STATUS_REG for new measurements
	uint8_t readSensor(IMU_OUT *dest); // Reads the measurements from the sensor and saves on dest
	uint8_t updConfigs(DRIVER_PARAMETER configs ); //Updates the settings
	void getReadings(IMU_OUT *dest);    //return the last requested sensor readings
    /**
     * @brief Return if the sensor is initialized on not
     */
    bool get_isInitialized(void);
    //=================================================================

private:
    // Variables
    //=================================================================
	IMU_CONFIGS settings;
	LSM6DSO_RAW raw_data;
	LSM6DSO_OUTPUT outputs;
	float ACCEL_SENSITIVITY;
	float GYRO_SENSITIVITY;
	DRIVER_PARAMETER parameters;

	// Defined in initialization
	I2C_Wrapper& i2c;
    bool isInitialized;		 // check this before using the driver to operate the IMU
    bool maxSampFreqReached; // if this is true check datasheet for maximum sampling frequency
	//=================================================================


	// Functions
	//=================================================================
	void processData(void); // Applies data processing on the raw data to convert it to pyhsical numbers
    void setSensitivity(void);
    uint8_t setSampFreq(void);


};







#endif /* LSM6DSO_DRIVER_LSM6DSO_HPP_ */
