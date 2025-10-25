/*
 * nau7802.cpp
 *
 * Implementation of the NAU7802 driver.
 */

#include "NAU7802.hpp"
#include "stm32f4xx_hal.h" // Only needed for HAL_Delay and HAL_GetTick

// Begin NAU7802 Startup Sequence
NAU7802::NAU7802(NAU7802_PARAMS configs, I2C_Wrapper& i2c_pointer)
  : i2c(i2c_pointer), parameters(configs) 
{
    isInitialized = false;
    i2c.updDeviceAddr(NAU7802_I2C_ADDRESS);

    // 1. Send a reset command
    if (!reset()) {
        return; // Failed to reset
    }
    HAL_Delay(10);

    // 2. Power up the analog and digital sections
    i2c.registerAddress = NAU7802_REG_PU_CTRL;
    i2c.sendData[0] = NAU7802_PU_CTRL_PUD | NAU7802_PU_CTRL_PUA;
    i2c.numBytes = 1;
    if (i2c.writeReg() != HAL_OK) {
        return; // Failed to write
    }
    
    // 3. Wait for the Power Up Ready bit
    uint32_t startTime = HAL_GetTick();
    while (HAL_GetTick() - startTime < 100) { // 100ms timeout
        i2c.registerAddress = NAU7802_REG_PU_CTRL;
        i2c.numBytes = 1;
        if (i2c.readReg() != HAL_OK) {
            return; // Failed to read
        }
        
        if ((i2c.receiveData[0] & NAU7802_PU_CTRL_PUR) != 0) {
            // Ready! Now set the initial gain.
            if (!setGain(parameters.initialGain)) {
                return; // Failed to set gain
            }
            
            isInitialized = true; // Success!
            return;
        }
        HAL_Delay(1);
    }
    
    // Timeout occurred
}

bool NAU7802::isReady() {
    i2c.registerAddress = NAU7802_REG_PU_CTRL;
    i2c.numBytes = 1;
    if (i2c.readReg() != HAL_OK) {
        return false;
    }
    return (i2c.receiveData[0] & NAU7802_PU_CTRL_CR) != 0;
}

uint8_t NAU7802::readSensor(NAU7802_OUT *dest) {
    i2c.registerAddress = NAU7802_REG_ADC_B2; // Start reading from MSB
    i2c.numBytes = 3; // Read 3 bytes
    uint8_t status = i2c.readReg();

    if (status != HAL_OK) {
        dest->raw_reading = 0;
        return status;
    }

    // Combine the three bytes from the wrapper's receiveData
    int32_t value = ((int32_t)i2c.receiveData[0] << 16) | \
                    ((int32_t)i2c.receiveData[1] << 8)  | \
                    (i2c.receiveData[2]);

    // Sign-extend the 24-bit value to a 32-bit integer
    if (value & 0x00800000) {
        value |= 0xFF000000;
    }

    dest->raw_reading = value;
    return status;
}

bool NAU7802::reset() {
    i2c.registerAddress = NAU7802_REG_PU_CTRL;
    i2c.sendData[0] = NAU7802_PU_CTRL_RR;
    i2c.numBytes = 1;
    if (i2c.writeReg() != HAL_OK) return false;
    
    HAL_Delay(1);
    
    i2c.sendData[0] = 0x00; // Clear reset bit
    if (i2c.writeReg() != HAL_OK) return false;
    
    return true;
}

bool NAU7802::setGain(uint8_t gain) {
    if (gain > NAU7802_GAIN_128X) {
        return false; // Invalid gain setting
    }
    
    // Read the current value of CTRL1
    i2c.registerAddress = NAU7802_REG_CTRL1;
    i2c.numBytes = 1;
    if (i2c.readReg() != HAL_OK) {
        return false;
    }

    uint8_t ctrl1_value = i2c.receiveData[0];
    ctrl1_value &= 0b11111000; // Clear the gain bits (bits 0, 1, 2)
    ctrl1_value |= gain;       // Set the new gain bits

    // Write the modified value back
    i2c.registerAddress = NAU7802_REG_CTRL1;
    i2c.sendData[0] = ctrl1_value;
    i2c.numBytes = 1;
    if (i2c.writeReg() != HAL_OK) {
        return false;
    }
    
    return true;
}

bool NAU7802::get_isInitialized(void) {
    return isInitialized;
}