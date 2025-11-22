/*
 * w25n04kvzeir.cpp
 *
 * Implementation of the w25n04kvzeir driver.
 */

#include "w25n04kvzeir.hpp"
#include "w25n04kvzeir_regs.hpp"
#include "spi_wrapper.hpp" 
#include <cstdint>

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

W25N04KVZEIR::W25N04KVZEIR(SPI_Wrapper* spiBus, GPIO_TypeDef* csPort, uint16_t csPin) :
    _spi(spiBus), 
    _csPort(csPort),
    _csPin(csPin) 
{

    // Constructor body.
    // Set the CS pin HIGH (idle) by default.
    HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_SET);
}

bool W25N04KVZEIR::begin(){
    uint8_t productID = getProductID();

    return (productID == W25N04KVZEIR_DID);
}

void W25N04KVZEIR::triggerMeasurement();

bool W25N04KVZEIR::readData();

void W25N04KVZEIR::performSet();

void W25N04KVZEIR::performReset();

std::uint8_t W25N04KVZEIR::getProductID(){
    return (readRegister(W25N04KVZEIR_DID));
}


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
 
/* w25n04kvzeir_CPP */