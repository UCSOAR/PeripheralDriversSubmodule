/*
 * spi_wrapper.cpp
 */
#include "spi_wrapper.hpp"

SPI_Wrapper::SPI_Wrapper(SPI_HandleTypeDef* hspi) : _hspi(hspi) {
}

std::uint8_t SPI_Wrapper::transfer(std::uint8_t txByte) {
    std::uint8_t rxByte = 0;
    
    // HAL to send one byte and receive one byte (txByte).
    HAL_SPI_TransmitReceive(_hspi, &txByte, &rxByte, 1, 100); // 100ms timeout
    
    return rxByte;
}

void SPI_Wrapper::transmit(std::uint8_t* data, std::uint16_t size) {
    // HAL to transmit a block of data.
    HAL_SPI_Transmit(_hspi, data, size, 100); // 100ms timeout
}