/*
 * nau7802.hpp
 *
 * Driver for the NAU7802 24-bit ADC.
 */
#ifndef NAU7802_HPP
#define NAU7802_HPP

#include <stdint.h>
#include "I2C_Wrapper.hpp"
#include "NAU7802_regs.hpp"

/**
 * @brief Configuration parameters for the NAU7802 driver.
 */
typedef struct NAU7802_DRIVER_PARAMETER {
    uint8_t initialGain; // Use one of the NAU7802_GAIN_xxx macros
} NAU7802_PARAMS;

/**
 * @brief Output data structure for the NAU7802.
 */
typedef struct NAU7802_OUTPUT {
    int32_t raw_reading;
} NAU7802_OUT;


class NAU7802 {
public:
    /**
     * @brief Constructs the NAU7802 Driver.
     * @param configs Configuration settings for the sensor.
     * @param i2c_pointer I2C Wrapper for communication.
     */
    NAU7802(NAU7802_PARAMS configs, I2C_Wrapper& i2c_pointer);

    // Check if conversion is ready
    bool isReady();

    // Read 24-bit signed ADC value
    uint8_t readSensor(NAU7802_OUT *dest);

    // Software reset
    bool reset();

    // Set PGA gain
    bool setGain(uint8_t gain);

    // Check if the driver is initialized
    bool get_isInitialized(void);

private:
    I2C_Wrapper& i2c; // Reference to the I2C wrapper
    bool isInitialized;
    NAU7802_PARAMS parameters; // Store the configs
};

#endif // NAU7802_HPP