/**
 * @file lsm6dso.hpp
 * @brief Implementation of the LSM6DSO driver.
 * @author Diogo Goto 
 * @version 1.0
 * @date 2025-02-01
 */

#ifndef LSM6DSO_DRIVER_LSM6DSO_HPP_
#define LSM6DSO_DRIVER_LSM6DSO_HPP_

#include "lsm6dso_regs.hpp" // Defines the address of some of the registers
#include "i2c_wrapper.hpp"  // I2C Wrapper for the communication with the sensor

// Structs
//=================================================================
/** 
 * @struct LSM6DSO_CONFIGS
 * @brief Internal configuration settings for the LSM6DSO sensor. 
 * @details This struct is used to store the configuration settings for the LSM6DSO sensor.
*/
typedef struct LSM6DSO_CONFIGS{
		uint8_t CTRL1_XL; // Set sampling frequency for Accelerometer
		uint8_t CTRL2_G;  // Set sampling frequency for Gyroscope
 		uint8_t CTRL6_C;  // High Performance Mode for Accelerometer
		uint8_t CTRL7_G;  // High Performance Mode for Gyroscope

		/*
*		uint8_t CTRL4_C;  // Ultra Low Power for Accelerometer
*		uint8_t CTRL5_C;  // Ultra Low Power for Gyroscope
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

/**
 * @struct DRIVER_PARAMETER
 * @brief Parameters for the operation LSM6DSO driver.
 * @details This struct should be created in the main file and used to constructor of the driver.
 * It is used to set the following parameters:
 * -	Maximum Gs to measure
 * -	Maximum dps to measure
 * -	Minimum needed accelerometer sampling frequency
 * -	Minimum needed gyroscope sampling frequency
 */
typedef struct DRIVER_PARAMETER{
	uint8_t maxG;			// Specify the maximum Gs to measure
	uint8_t maxDPS;			// Specify the maximum dps to measure
	uint16_t accelMinFreq;  // Specify the minimum needed accelerometer sampling frequency
	uint16_t gyroMinFreq;	// Specify the minimum needed gyroscope sampling frequency
}PARAMETERS;

/**
 * @struct LSM6DSO_RAW
 * @brief Raw data from the LSM6DSO sensor.
 * @details This struct is used to store the raw data from the LSM6DSO sensor.
 */
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

/**
 * @struct LSM6DSO_OUTPUT
 * @brief Processed data from the LSM6DSO sensor.
 * @details This struct is used to store the processed data from the LSM6DSO sensor.
 */
typedef struct LSM6DSO_OUTPUT{
	float temp; // in Celcius

	float gyro_x; // in G
	float gyro_y; // in G
	float gyro_z; // in G

	float accel_x; //in dps
	float accel_y; //in dps
	float accel_z; //in dps
} IMU_OUT;

//=================================================================


// Class Definition
//=================================================================
/**
 * @class LSM6DSO_Driver
 * @brief Driver for the LSM6DSO sensor.
 * @details This class is used to control the LSM6DSO sensor. It is used to configure the sensor and read the measurements.
 */
class LSM6DSO_Driver{
public:
	//Constructor
	//=================================================================
	/**
	* @brief Constructs the LSM6_DSO Driver.
	* @param[in] configs Configuration settings for the sensor.
	* @param[in] i2c_pointer I2C Wrapper for the communication with the sensor.
	*/
	LSM6DSO_Driver(DRIVER_PARAMETER configs, I2C_Wrapper& i2c_pointer);
	//=================================================================

	//Function Definitions
	//=================================================================
	// Read data
	/**
	 * @brief //Reads the STATUS_REG for new measurements available.
	 * @details This function reads the STATUS_REG for new measurements available and updates the status variable in the raw_data struct in the driver.
	 * @return Returns if the reading was successful or not.
	 */
	uint8_t updStatus(void);
	
	/**
	 * @brief Reads the measurements from the sensor and saves on dest and in the outputs member struct.
	 * @details This function reads the measurements from the sensor and saves on dest and in the outputs member struct.
	 * @param[out] dest Pointer to the struct where the measurements will be saved.
	 * @return Returns if the reading was successful or not.
	 */   
	uint8_t readSensor(IMU_OUT *dest); // Reads the measurements from the sensor and saves on dest
	
	/**
	 * @brief Returns the last requested sensor readings.
	 * @details This function returns the last requested sensor readings.
	 * @param[out] dest Pointer to the struct where the measurements will be saved.
	 */
	void getReadings(IMU_OUT *dest);    //return the last requested sensor readings

	//Simplified Configurations

	/**
	 * @brief Returns if the sensor is initialized on not.
	 * @return True if the sensor is initialized, false otherwise.
	*/
    bool get_isInitialized(void);

	/**
	 * @brief Updates the settings.
	 * @details Based on the entered parameters, this function updates the settings of the sensor.
	 * @param[in] configs New operation parameters settings for the sensor.
	 * @return Returns if the update was successful or not.
	 */
	uint8_t updConfigs(DRIVER_PARAMETER configs ); //Updates the settings


	//Complete Configurations (use this if you know what you're doing)
	/**
	 * @brief Updates the CTRL1_XL register.
	 * @details This function updates the CTRL1_XL (Accel sampling frequency and full scale range) register in the sensor.
	 * @param[in] newVal New value for the CTRL1_XL register.
	 * @return Returns if the update was successful or not.
	 */
	uint8_t updCTRL1_XL(uint8_t newVal);

	/**
	 * @brief Updates the CTRL2_G register.
	 * @details This function updates the CTRL2_G (Gyro sampling frequency and full scale range) register in the sensor.
	 * @param[in] newVal New value for the CTRL2_G register.
	 * @return Returns if the update was successful or not.
	 */
	uint8_t updCTRL2_G(uint8_t newVal);

	//uint8_t updCTRL4_C(uint8_t newVal);
	//uint8_t updCTRL5_C(uint8_t newVal);
	
	/**
	 * @brief Updates the CTRL6_C register.
	 * @details This function updates the CTRL6_C (High Performance Mode for Accelerometer) register in the sensor.
	 * @param[in] newVal New value for the CTRL6_C register.
	 * @return Returns if the update was successful or not.
	 */
	uint8_t updCTRL6_C(uint8_t newVal);
	
	/**
	 * @brief Updates the CTRL7_G register.
	 * @details This function updates the CTRL7_G (High Performance Mode for Gyroscope) register in the sensor.
	 * @param[in] newVal New value for the CTRL7_G register.
	 * @return Returns if the update was successful or not.
	 */
	uint8_t updCTRL7_G(uint8_t newVal);
	
	//uint8_t updCTRL8_XL(uint8_t newVal);
	//uint8_t updCTRL10_C(uint8_t newVal);
	//uint8_t upd(uint8_t newVal);

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
	/**
	 * @brief Applies data processing on the raw data to convert it to physical numbers.
	 * @details This function applies data processing on the raw data to convert it to physical numbers and saves in the outputs.
	 */
	void processData(void); // Applies data processing on the raw data to convert it to physical numbers

	/**
	 * @brief Sets the sensitivity of the sensor.
	 * @details This function sets the sensitivity of the sensor based on the maximum Gs and dps to measure in the entered parameters.
	*/
    void setSensitivity(void);

	/**
	 * @brief Sets the sampling frequency of the sensor.
	 * @details This function sets the sampling frequency of the sensor based on the minimum needed accelerometer and gyroscope sampling frequency in the entered parameters.
	 * @return Returns if the set was successful or not.
	 * @note This function is used in the constructor to set the sampling frequency of the sensor. This function is used in the updConfigs function to update the sampling frequency of the sensor.
	*/
	uint8_t setSampFreq(void);
	//=================================================================


};
#endif /* LSM6DSO_DRIVER_LSM6DSO_HPP_ */
