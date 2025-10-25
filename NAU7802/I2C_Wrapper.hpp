/*
 * i2c_wrapper.hpp
 */

#ifndef I2C_WRAPPER_HPP_
#define I2C_WRAPPER_HPP_

#include <array>

extern "C" {
    #include "stm32f4xx_hal.h" 
}

#define COMM_ERROR 5

class I2C_Wrapper {
public:
    // Constructor
    I2C_Wrapper(I2C_HandleTypeDef* i2c)
        : hi2c(i2c), deviceAddress(0) {}

    // Variables
    uint8_t registerAddress;
    uint8_t numBytes;
    std::array<uint8_t, 254> sendData;
    std::array<uint8_t, 255> receiveData;

    // Functions
    uint8_t writeReg();
    uint8_t readReg();
    void updDeviceAddr(uint8_t newAddress);

private:
    I2C_HandleTypeDef* hi2c;
    uint8_t deviceAddress;
};

#endif /* I2C_WRAPPER_HPP_ */