#ifndef MAX31856MUD_DRIVER_HPP_
#define MAX31856MUD_DRIVER_HPP_

#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <assert.h>

#define SPI_DUMMY_BYTE 0xFF

MAX31856Driver::MAX31856Driver() {
}

MAX31856Driver::~MAX31856Driver() {
}

void MAX31856Driver::Init(SPI_HandleTypeDef* hspi_, GPIO_TypeDef* cs_gpio_, uint16_t cs_pin_) {
    hspi = hspi_;
    initialized = true;

    SetCSPin(cs_gpio_, cs_pin_);
    CSHigh();

    // Optionally read CR0 to confirm SPI works
    (void)GetRegister(MAX31856_REG::CR0);
}

void MAX31856Driver::CSLow() {
    assert(initialized);
    HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_RESET);
}

void MAX31856Driver::CSHigh() {
    assert(initialized);
    HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_SET);
}

void MAX31856Driver::SetCSPin(GPIO_TypeDef* gpio, uint16_t pin) {
    cs_gpio = gpio;
    cs_pin = pin;
}

bool MAX31856Driver::SetRegister(MAX31856_REG::REG reg, uint8_t val) {
    assert(initialized);

    uint8_t tx[2] = {
        uint8_t(reg & 0x7F),
        val
    };

    CSLow();
    HAL_StatusTypeDef r = HAL_SPI_Transmit(hspi, tx, 2, 1000);
    CSHigh();

    return r == HAL_OK;
}

uint8_t MAX31856Driver::GetRegister(MAX31856_REG::REG reg) {
    assert(initialized);

    uint8_t tx[2] = {
        uint8_t(reg | 0x80), 
        SPI_DUMMY_BYTE
    };

    uint8_t rx[2] = {0,0};

    CSLow();
    HAL_SPI_TransmitReceive(hspi, tx, rx, 2, 1000);
    CSHigh();

    return rx[1];
}

void MAX31856Driver::GetMultipleRegisters(MAX31856_REG::REG startreg, int numBytes, uint8_t* out) {
    assert(initialized);

    uint8_t tx[numBytes + 1];
    uint8_t rx[numBytes + 1];

    tx[0] = startreg | 0x80;  
    for(int i = 1; i <= numBytes; i++) tx[i] = SPI_DUMMY_BYTE;

    CSLow();
    HAL_SPI_TransmitReceive(hspi, tx, rx, numBytes + 1, 1000);
    CSHigh();

    // skip first (command echo)
    for(int i = 0; i < numBytes; i++)
        out[i] = rx[i+1];
}

//thermo couple tempature
float MAX31856Driver::ReadThermocoupleTempC() {
    uint8_t buf[3];
    GetMultipleRegisters(MAX31856_REG::LTCBH, 3, buf);

    // Convert 24-bit 
    int32_t raw = (buf[0] << 16) | (buf[1] << 8) | buf[2];

    // Sign extend
    if(raw & 0x800000)
        raw |= 0xFF000000;

    return raw / 1024.0f;
}

uint8_t MAX31856Driver::GetFaultStatus() {
    return GetRegister(MAX31856_REG::SR);
}