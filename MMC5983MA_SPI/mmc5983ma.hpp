/*
 * mmc5983ma.hpp
 *
 * C++ driver for the MMC5983MA magnetometer.
 */

#ifndef MMC5983MA_HPP
#define MMC5983MA_HPP

#include "mmc5983ma_regs.hpp"
#include "spi_wrapper.hpp" 
#include <cstdint>
// if needed: fw declaration for the SPIClass from HAL/SPI wrapper, 
// class SPIClass; 
extern "C" {
    #include "stm32f4xx_hal.h"
}

struct MagData {
    std::uint32_t rawX;
    std::uint32_t rawY;
    std::uint32_t rawZ;
    float scaledX;
    float scaledY;
    float scaledZ;
};

// New Status Enum
enum class MMC5983MA_Status : uint8_t {
    OK = 0,
    ERR_I2C,
    ERR_TIMEOUT,
    ERR_NOT_READY,
    ERR_INVALID_ARG
};
class MMC5983MA {
public:
    /**
     * @brief Constructor
     * @param spiBus Pointer to an initialized SPI wrapper instance.
     * @param csPin The GPIO pin for chip select.
     */
    MMC5983MA(SPI_Wrapper* spiBus, GPIO_TypeDef* csPort, std::uint16_t csPin);
    
    /**
     * @brief Initializes the sensor.
     * @return True on success (e.g., product ID matches), false otherwise.
     */
    MMC5983MA_Status begin();

    /**
     * @brief Triggers a new magnetic field measurement.
     */
    MMC5983MA_Status triggerMeasurement();

    /**
     * @brief Reads the latest magnetic field data from the sensor.
     * @return True if data is ready and read, false otherwise.
     */
    MMC5983MA_Status readData(MagData& data);

    /**
     * @brief Performs a SET operation.
     */
    MMC5983MA_Status performSet();

    /**
     * @brief Performs a RESET operation.
     */
    MMC5983MA_Status performReset();

    /**
     * @brief Reads the product ID register.
     * @return The 8-bit product ID, or 0 on failure.
     */
    std::uint8_t getProductID();

    // --- More functions; later ---
    // bool isDataReady();
    // void setBandwidth(std::uint8_t bw);
    // float getTemperature();
    // void startContinuousMode(std::uint8_t freq);
    // void stopContinuousMode();

    /* 
     * @brief Calibrates the sensor to determine null field offset.
     * @return Status of the calibration operation.
     * WARNING: This function BLOCKS for ~25 ms to perform two measurements.
    */
    MMC5983MA_Status calibrateOffset();
    
    /**
     * @brief Sets user-defined offsets for X, Y, Z axes.
     * @param offsetX Offset for X axis.
     * @param offsetY Offset for Y axis.
     * @param offsetZ Offset for Z axis.
     * @return Status of the operation.
     */
    MMC5983MA_Status setOffsets(float offsetX, float offsetY, float offsetZ);
    
    /**
     * @brief Reads the temperature data from the sensor.
     * @param tempOut Reference to store the temperature in degrees Celsius.
     * @return Status of the operation.
     */
    MMC5983MA_Status readTemperature(float& tempOut);

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

    // Constants for scaling data
    const float _countsPerGauss = 16384.0f;
    // Null field offsets
    float _offsetX = 131072.0f;
    float _offsetY = 131072.0f;
    float _offsetZ = 131072.0f;


};

#endif // MMC5983MA_HPP