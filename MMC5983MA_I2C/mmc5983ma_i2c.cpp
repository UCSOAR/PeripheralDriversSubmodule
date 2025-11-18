/*
 * mmc5983ma.cpp
 *
 * Implementation of the MMC5983MA driver.
 */

#include "mmc5983ma_i2c.hpp"
#include "mmc5983ma_regs.hpp"
#include "i2c_wrapper.hpp" 
#include <cstdint>

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;


// Constructor
MMC5983MA::MMC5983MA(I2C_Wrapper* i2cBus, uint8_t adress) : _i2c(i2cBus) {
    _address = (adress << 1); // left shift for HAL compatibility
}

bool MMC5983MA::begin(){
    uint8_t productID = getProductID();

    return (productID == MMC5983MA_PRODUCT_ID_VALUE);
}

uint8_t MMC5983MA::getProductID(){
    // (P ID at 0x2F)
    return (readRegister(MMC5983MA_P_ID));
}


void MMC5983MA::triggerMeasurement(){
    writeRegister(MMC5983MA_IT_CONTROL0, MMC5983MA_TM_M);
}

void MMC5983MA::performSet(){
    writeRegister(MMC5983MA_IT_CONTROL0, MMC5983MA_SET);
}


void MMC5983MA::performReset(){
    writeRegister(MMC5983MA_IT_CONTROL0, MMC5983MA_RESET);
}





bool MMC5983MA::readData(MagData& data) {
    // check Status reg
    uint8_t status = readRegister(MMC5983MA_STATUS);
    if (!(status & MMC5983MA_MEAS_M_DONE)) {
        return false; // Data not ready
    }
    
    // Read 7 measurement regs at once.
    uint8_t buffer[7];
    readRegisters(MMC5983MA_XOUT0, buffer, 7);

    // Combine bytes into raw 18-bit values
    data.rawX = ((uint32_t)buffer[0] << 10) |
                ((uint32_t)buffer[1] << 2)  |
                ((uint32_t)(buffer[6] & 0xC0) >> 6);

    data.rawY = ((uint32_t)buffer[2] << 10) |
                ((uint32_t)buffer[3] << 2)  |
                ((uint32_t)(buffer[6] & 0x30) >> 4);

    data.rawZ = ((uint32_t)buffer[4] << 10) |
                ((uint32_t)buffer[5] << 2)  |
                ((uint32_t)(buffer[6] & 0x0C) >> 2);


    // Apply scaling factors (Gauss)
    data.scaledX = ((float)data.rawX - _nullFieldOffset) / _countsPerGauss;
    data.scaledY = ((float)data.rawY - _nullFieldOffset) / _countsPerGauss;
    data.scaledZ = ((float)data.rawZ - _nullFieldOffset) / _countsPerGauss;

        return true;
}



/* ------------- PRIVATE HELPERS ------------- */

void MMC5983MA::writeRegister(uint8_t reg, uint8_t value) {
    _i2c->writeByte(_address, reg, value);
}

uint8_t MMC5983MA::readRegister(uint8_t reg) {
    return _i2c->readByte(_address, reg);
}

void MMC5983MA::readRegisters(uint8_t reg, uint8_t* buffer, uint8_t len) {
    _i2c->readBytes(_address, reg, buffer, len);
}

/* MMC5983MA_CPP */