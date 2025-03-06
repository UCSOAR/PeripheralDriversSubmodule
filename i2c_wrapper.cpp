/*
 * i2c_wrapper.cpp
 *
 *  Created on: Nov 16, 2024
 *      Author: diogo
 */


/* Includes ------------------------------------------------------------------*/
#include "i2c_wrapper.hpp"



/** Member Function Implementations ------------------------------------------*/

// Write to a register
uint8_t I2C_Wrapper::writeReg() {
    uint8_t status;
    // Create a std::array to store the data to be sent
       std::array<uint8_t, 255> tempSend = {0};  // Size 255 for register Address + data
       tempSend[0] = registerAddress;

       // Copy the data into tempSend array starting from index 1
       std::copy(sendData.begin(), sendData.begin() + numBytes  , tempSend.begin() + 1);

    // Send the register address and data
    status = HAL_I2C_Master_Transmit(hi2c, deviceAddress, tempSend.data(), numBytes + 1, 1000);
    if (status != HAL_OK)
        return status;

    // Confirm the data
    status = HAL_I2C_Master_Transmit(hi2c, deviceAddress, &registerAddress, 1, 1000); // Send the register address again
    if (status != HAL_OK)
        return status;

    uint8_t receivedData[255];
    status = HAL_I2C_Master_Receive(hi2c, deviceAddress, receivedData, numBytes, 1000); // Read back data
    if (status != HAL_OK)
        return status;

    // Verify the data
    for (int i = 0; i < numBytes; ++i) {
        if (sendData[i] != receivedData[i])
            return COMM_ERROR; // Data mismatch
    }

    return HAL_OK; // Success
}

// Read from a register
uint8_t I2C_Wrapper::readReg() {
    uint8_t status;

    // Send the register address
    status = HAL_I2C_Master_Transmit(hi2c, deviceAddress, &registerAddress, 1, 1000);
    if (status != HAL_OK)
        return status;

    // Read the data from the register
    status = HAL_I2C_Master_Receive(hi2c, deviceAddress, receiveData.data(), numBytes, 1000);
    return status;
}


void I2C_Wrapper::updDeviceAddr(uint8_t newAddress){
	deviceAddress = newAddress;
}

