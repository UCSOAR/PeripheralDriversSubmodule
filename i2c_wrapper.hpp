/*
 * i2c_wrapper.hpp
 *
 *  Created on: Nov 16, 2024
 *      Author: diogo
 */

#ifndef I2C_WRAPPER_HPP_
#define I2C_WRAPPER_HPP_

#include <array>

extern "C" {
    #include "stm32h7xx_hal.h"
}

#define COMM_ERROR 5


/**
 * @brief Wrapper for I2C communication with specific device.
 */
class I2C_Wrapper {
public:
	//Constructor
	//=================================================================
    /**
     * @brief Constructs the I2C_Wrapper object.
     * @param i2c Pointer to the I2C handle.
     * @param deviceAddress 8-bit device address.
     */
	I2C_Wrapper(I2C_HandleTypeDef* i2c, uint8_t deviceAddress)
	        : hi2c(i2c), deviceAddress(deviceAddress) {}
	//=================================================================


	//Variables
	//=================================================================
	uint8_t 			     registerAddress;
	uint8_t 				 numBytes;        // Don't include the byte for the register address
	std::array<uint8_t, 254> sendData;        // 254 + 1 for the register address
	std::array<uint8_t, 255> receiveData;
	//=================================================================



    // Functions
	//=================================================================
	/**
	 * @brief Writes data to a register on the I2C device.
	 *
	 * This function writes the data stored in the `sendData` buffer to the
	 * register specified by `registerAddress` on the I2C device.
	 *
	 * @return uint8_t 0 if success, non-zero if an error occurs.
	 */
    uint8_t writeReg();



    /**
     * @brief Reads data from a register on the I2C device.
     *
     * This function reads data from the register specified by `registerAddress`
     * on the I2C device into the `receiveData` buffer.
     *
     * @return uint8_t 0 if success, non-zero if an error occurs.
     */
    uint8_t readReg();


    /**
     * @brief Update the device address variable
     *
     * @param newAddress 8-bits long address of the new device.
     *
     */
    void updDeviceAddr(uint8_t newAddress);


    //=================================================================

private:
    I2C_HandleTypeDef* hi2c;
    uint8_t deviceAddress;
};



#endif /* I2C_WRAPPER_HPP_ */
