/*
 * spi_wrapper.cpp
 */
#include "spi_wrapper.hpp"

SpiWrapper::SpiWrapper(SPI_HandleTypeDef* hspi) : _hspi(hspi) {
}

std::uint8_t SpiWrapper::transfer(std::uint8_t txByte) {
    std::uint8_t rxByte = 0;
    
    // HAL to send one byte and receive one byte (txByte).
    HAL_SPI_TransmitReceive(_hspi, &txByte, &rxByte, 1, 1000); // 1000ms timeout
    
    return rxByte;
}

void SpiWrapper::transmit(std::uint8_t* data, std::uint16_t size) {
    // HAL to transmit a block of data.
    HAL_SPI_Transmit(_hspi, data, size, 1000); // 1000ms timeout
}