/*
 * spi_wrapper.hpp
 */

#ifndef SPI_WRAPPER_HPP
#define SPI_WRAPPER_HPP

#include <array>

extern "C" {
    #include "stm32f4xx_hal.h"
}

#define COMM_ERROR 5

class SPI_Wrapper {
public:
    // Constructor
    SPI_Wrapper(SPI_HandleTypeDef* spi)
        : hspi(spi) {}

    // Variables
    std::uint8_t registerAddress;
    std::uint8_t numBytes;
    std::array<std::uint8_t, 254> sendData;
    std::array<std::uint8_t, 255> receiveData;

    // Functions
    std::uint8_t writeReg();
    std::uint8_t readReg();
    void updDeviceAddr(std::uint8_t newAddress);

private:
    SPI_HandleTypeDef* hspi;
    std::uint8_t deviceAddress;
};

#endif /* SPI_WRAPPER_HPP_ */