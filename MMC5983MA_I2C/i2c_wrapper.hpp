/*
 * i2c_wrapper.hpp
 */

#ifndef I2C_WRAPPER_HPP
#define I2C_WRAPPER_HPP

#include <cstdint>

extern "C" {
    #include "stm32f4xx_hal.h"
}

//#define COMM_ERROR 5

class I2C_Wrapper {
public:
    /**
     * @brief Constructor.
     * @param hi2c Pointer to the HAL I2C handle (e.g., &hi2c1)
     */

    I2C_Wrapper(I2C_HandleTypeDef* hi2c);

    /**
     * @brief Transmits and receives a single byte.
     * @param txByte The byte to send.
     * @return The byte received.
     */
    std::uint8_t transfer(std::uint8_t txByte);

    /**
     * @brief Transmits a block of data.
     * @param data Pointer to the data to send.
     * @param size Number of bytes to send.
     */
    void transmit(std::uint8_t* data, std::uint16_t size);
    
private:
    I2C_HandleTypeDef* _hi2c;
};

#endif // I2C_WRAPPER_HPP