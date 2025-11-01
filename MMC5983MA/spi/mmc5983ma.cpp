/*
 * mmc5983ma.cpp
 *
 * Implementation of the MMC5983MA driver.
 */
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

    MMC5983MA(spi_wrapper* spiBus, 
        uint16_t csPin)

    bool begin();

    void triggerMeasurement();

    bool readData(MagData& data);


    void performSet();

    void performReset();

    std::uint8_t getProductID();

    // --- More functions; later ---
    // bool isDataReady();
    // void setBandwidth(std::uint8_t bw);
    // float getTemperature();
    // void startContinuousMode(std::uint8_t freq);
    // void stopContinuousMode();


private:

    void writeRegister(std::uint8_t reg, std::uint8_t value);


    std::uint8_t readRegister(std::uint8_t reg);


    void readRegisters(std::uint8_t reg, std::uint8_t* buffer, std::uint8_t len);
    SpiWrapper* _spi;
    std::uint16_t _csPin;
 
};

#endif // MMC5983MA_HPP