/*
 * i2c_wrapper.cpp
 */
#include "i2c_wrapper.hpp"

I2C_Wrapper::I2C_Wrapper(I2C_HandleTypeDef* hi2c) : _hi2c(hi2c) {
}

std::uint8_t I2C_Wrapper::transfer(std::uint8_t txByte) {
    std::uint8_t rxByte = 0;

    // HAL to send one byte and receive one byte (txByte).
    HAL_I2C_Master_TransmitReceive(_hi2c, &txByte, &rxByte, 1, 100); // 100ms timeout

    return rxByte;
}

void I2C_Wrapper::transmit(std::uint8_t* data, std::uint16_t size) {
    // HAL to transmit a block of data.
    HAL_I2C_Master_Transmit(_hi2c, data, size, 100); // 100ms timeout
}
