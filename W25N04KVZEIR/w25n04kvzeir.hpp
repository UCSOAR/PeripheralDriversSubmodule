/*
 * x.hpp
 *
 * C++ driver for the x magnetometer.
 */

#ifndef W25N04KVZEIR_HPP
#define W25N04KVZEIR_HPP

#include "w25n04kvzeir_regs.hpp"
#include "spi_wrapper.hpp" 
#include <cstdint>

extern "C" {
    #include "stm32f4xx_hal.h"
}

class W25N04KVZEIR {
public:
    /**
     * @brief Constructor
     * @param spiBus Pointer to an initialized SPI wrapper instance.
     * @param csPin The GPIO pin for chip select.
     */
    W25N04KVZEIR(SPI_Wrapper* spiBus, GPIO_TypeDef* csPort, std::uint16_t csPin);
    
    /**
     * @brief Initializes the sensor.
     * @return True on success (e.g., product ID matches), false otherwise.
     */
    bool begin();

    /**
     * @brief Triggers a new magnetic field measurement.
     */
    void triggerMeasurement();

    /**
     * @brief Reads the latest magnetic field data from the sensor.
     * @return True if data is ready and read, false otherwise.
     */
    bool readData();

    /**
     * @brief Performs a SET operation.
     */
    void performSet();

    /**
     * @brief Performs a RESET operation.
     */
    void performReset();

    /**
     * @brief Reads the product ID register.
     * @return The 8-bit product ID, or 0 on failure.
     */
    std::uint8_t getProductID();

private:
    /**
     * @brief Writes a single byte to a sensor register.
     * @param reg The register address.
     * @param value The 8-bit value to write.
     */
    void writeRegister(std::uint8_t reg, std::uint8_t value);

    /**
     * @brief Reads a single byte from a sensor register.
     * @param reg The register address.
     * @return The 8-bit value read.
     */
    std::uint8_t readRegister(std::uint8_t reg);

    /**
     * @brief Reads multiple bytes from the sensor starting at a register.
     * @param reg The starting register address.
     * @param buffer Pointer to the buffer to store read data.
     * @param len Number of bytes to read.
     */
    void readRegisters(std::uint8_t reg, std::uint8_t* buffer, std::uint8_t len);
    
    // Member variables
    SPI_Wrapper* _spi;
    std::uint16_t _csPin;
    GPIO_TypeDef* _csPort;

};

#endif // W25N04KVZEIR_HPP