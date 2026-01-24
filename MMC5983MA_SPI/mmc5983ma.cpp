/*
 * mmc5983ma.cpp
 *
 * Implementation of the MMC5983MA driver.
 */

#include "mmc5983ma.hpp"
#include "mmc5983ma_regs.hpp"
#include "spi_wrapper.hpp" 
#include <cstdint>

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

/**
 * @brief Constructor
 */
MMC5983MA::MMC5983MA(SPI_Wrapper* spiBus, GPIO_TypeDef* csPort, uint16_t csPin) :
    _spi(spiBus), 
    _csPort(csPort),
    _csPin(csPin) 
{
    // Constructor body.
    // Set the chip select pin HIGH (idle) by default.
    HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_SET);
}

MMC5983MA_Status MMC5983MA::begin(){
    uint8_t productID = getProductID();

    if (productID == MMC5983MA_PRODUCT_ID_VALUE) {
        return MMC5983MA_Status::OK;
    }
    else {
        return MMC5983MA_Status::ERR_INVALID_ARG;
    }
}

uint8_t MMC5983MA::getProductID(){
    // (P ID at 0x2F)
    return (readRegister(MMC5983MA_P_ID));
}


MMC5983MA_Status MMC5983MA::triggerMeasurement(){
    writeRegister(MMC5983MA_IT_CONTROL0, MMC5983MA_TM_M);
}

MMC5983MA_Status MMC5983MA::performSet(){
    writeRegister(MMC5983MA_IT_CONTROL0, MMC5983MA_SET);
}


MMC5983MA_Status MMC5983MA::performReset(){
    writeRegister(MMC5983MA_IT_CONTROL0, MMC5983MA_RESET);
}





MMC5983MA_Status MMC5983MA::readData(MagData& data) {
    // Read status register to check if data is ready
    uint8_t status = readRegister(MMC5983MA_STATUS);

    // Check if measurement done bit is set
    if (!(status & MMC5983MA_MEAS_M_DONE)) {
        return MMC5983MA_Status::ERR_NOT_READY; // Data not ready
    }
    
    // Data Ready. Read all 7 measurement regs at once.
    uint8_t buffer[7];
    readRegisters(MMC5983MA_XOUT0, buffer, 7);

    data.rawX = ((uint32_t)buffer[0] << 10) |
                ((uint32_t)buffer[1] << 2)  |
                ((uint32_t)(buffer[6] & 0xC0) >> 6);

    data.rawY = ((uint32_t)buffer[2] << 10) |
                ((uint32_t)buffer[3] << 2)  |
                ((uint32_t)(buffer[6] & 0x30) >> 4);

    data.rawZ = ((uint32_t)buffer[4] << 10) |
                ((uint32_t)buffer[5] << 2)  |
                ((uint32_t)(buffer[6] & 0x0C) >> 2);


        // Apply scaling factors
        data.scaledX = ((float)data.rawX - _nullFieldOffset) / _countsPerGauss;
        data.scaledY = ((float)data.rawY - _nullFieldOffset) / _countsPerGauss;
        data.scaledZ = ((float)data.rawZ - _nullFieldOffset) / _countsPerGauss;

        return MMC5983MA_Status::OK;
}



/* ========================================================================*/
/* ========================PRIVATE HELPER FUNCTIONS========================*/
/* ========================================================================*/

void MMC5983MA::writeRegister(std::uint8_t reg, std::uint8_t value) {
    // Write : R/W bit (0) == 0
    uint8_t cmd_byte = (reg << 2) & 0xFC;
    uint8_t txBuffer[2] = { cmd_byte, value };

    // Pull cd Low to select the chip
    HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_RESET);

    // Use our wrapper to transmit the 2 bytes
    _spi->transmit(txBuffer, 2);

    // Pull CS High to deselect the chip
    HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_SET);
}

uint8_t MMC5983MA::readRegister(uint8_t reg){
    // Read : R/W bit (0) == 1
    // Shift address left 2 bits, then OR with 0x01 to set the Read bit
    uint8_t cmd_byte = ((reg << 2) & 0xFC) | 0x01;

    // Pull CS Low
    HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_RESET);

    // 1. Send the command byte
    _spi->transfer(cmd_byte);

    // 2. Send dummy byte (0x00) to clock out the data
    uint8_t rx_value = _spi->transfer(0x00);

    // Pull CS High
    HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_SET);

    return rx_value;
}

/**
 * @brief Reads multiple bytes from the sensor.
 */
void MMC5983MA::readRegisters(std::uint8_t reg, std::uint8_t* buffer, std::uint8_t len) {
    // 1. Create the command byte (same as readRegister)
    uint8_t cmd_byte = ((reg << 2) & 0xFC) | 0x01;

    // Pull CS Low
    HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_RESET);

    // 2. Send the command/address byte
    _spi->transfer(cmd_byte);
    
    // 3. Read 'len' bytes into buffer
    for (uint8_t i = 0; i < len; ++i) {
        buffer[i] = _spi->transfer(0x00); // Send dummy byte to clock out data
    }

    // Pull CS High
    HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_SET);


}
/* MMC5983MA_CPP */