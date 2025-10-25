/*
 * i2c_wrapper.cpp
 */

#include "I2C_Wrapper.hpp"

/**
 * @brief Writes data to a register on the I2C device.
 * @note Uses HAL_I2C_Mem_Write
 */
uint8_t I2C_Wrapper::writeReg() {
    if (numBytes != 1) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
        hi2c,
        deviceAddress,
        registerAddress,
        I2C_MEMADD_SIZE_8BIT,
        sendData.data(),
        numBytes,
        100 // timeout
    );
    
    return (uint8_t)status;
}

/**
 * @brief Reads data from a register on the I2C device.
 * @note Uses HAL_I2C_Mem_Read
 */
uint8_t I2C_Wrapper::readReg() {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        hi2c,
        deviceAddress,
        registerAddress,
        I2C_MEMADD_SIZE_8BIT,
        receiveData.data(),
        numBytes,
        100 // timeout
    );

    return (uint8_t)status;
}

void I2C_Wrapper::updDeviceAddr(uint8_t newAddress) {
    deviceAddress = newAddress;
}