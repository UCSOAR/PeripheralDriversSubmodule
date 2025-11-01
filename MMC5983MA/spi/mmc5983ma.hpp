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

struct MagData {
    std::uint32_t rawX;
    std::uint32_t rawY;
    std::uint32_t rawZ;
    float scaledX;
    float scaledY;
    float scaledZ;
};

class MMC5983MA {
public:
    /**
     * @brief Constructor
     * @param spiBus Pointer to an initialized SPI wrapper instance.
     * @param csPin The GPIO pin for chip select.
     */
    MMC5983MA(SpiWrapper* spiBus, std::uint16_t csPin); // Assuming SpiWrapper and CS pin
    
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
    bool readData(MagData& data);

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

    // --- More functions; later ---
    // bool isDataReady();
    // void setBandwidth(std::uint8_t bw);
    // float getTemperature();
    // void startContinuousMode(std::uint8_t freq);
    // void stopContinuousMode();


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
    SpiWrapper* _spi;
    std::uint16_t _csPin;

    // sensitivity here for scaling raw data
    // float _sensitivity; 
};

#endif // MMC5983MA_HPP