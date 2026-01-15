#ifndef KSZ9893R_DRIVER_HPP_
#define KSZ9893R_DRIVER_HPP_

#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <assert.h>

#define SPI_DUMMY_BYTE 0xFF


void KSZ9893RDriver::Init(SPI_HandleTypeDef* hspi_, GPIO_TypeDef* gpio, uint16_t pin) {
    hspi = hspi_;
    cs_gpio = gpio;
    cs_pin = pin;
    initialized = true;

    CSHigh();

    // Verify chip ID
    uint8_t id1 = ReadRegister(KSZ9893R_REG::CHIP_ID1);
    uint8_t id2 = ReadRegister(KSZ9893R_REG::CHIP_ID2);

    if(id1 != KSZ9893R_CHIP_ID1_VAL || id2 != KSZ9893R_CHIP_ID2_VAL)
        initialized = false;
}

bool KSZ9893RDriver::WriteRegister(uint16_t addr, uint8_t val) {
    assert(initialized);

    uint8_t tx[4];
    tx[0] = KSZ_SPI_WRITE | ((addr >> 9) & 0x7F);
    tx[1] = (addr >> 1) & 0xFF;
    tx[2] = (addr & 0x01) << 7;
    tx[3] = val;

    CSLow();
    HAL_StatusTypeDef status = HAL_SPI_Transmit(hspi, tx, 4, HAL_MAX_DELAY);
    CSHigh();

    return status == HAL_OK;
}

uint8_t KSZ9893RDriver::ReadRegister(uint16_t addr) {
    assert(initialized);

    uint8_t tx[4];
    uint8_t rx[4] = {0};

    tx[0] = KSZ_SPI_READ | ((addr >> 9) & 0x7F);
    tx[1] = (addr >> 1) & 0xFF;
    tx[2] = (addr & 0x01) << 7;
    tx[3] = SPI_DUMMY_BYTE;

    CSLow();
    HAL_SPI_TransmitReceive(hspi, tx, rx, 4, HAL_MAX_DELAY);
    CSHigh();

    return rx[3];
}

void KSZ9893RDriver::ReadMultipleRegisters(uint16_t startreg, int numBytes, uint8_t *out) {
    assert(initialized);
    for(int i = 0; i < numBytes; i++) {
        out[i] = ReadRegister(startreg + i);
    }
}

void KSZ9893RDriver::CSLow() {
    assert(initialized);
    HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_RESET);
}

void KSZ9893RDriver::CSHigh() {
    assert(initialized);
    HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_SET);
}

void KSZ9893RDriver::SetCSPin(GPIO_TypeDef* gpio, uint16_t pin) {
    cs_gpio = gpio;
    cs_pin = pin;
}


bool KSZ9893RDriver::SoftReset() {
    uint8_t reg = ReadRegister(SWITCH_OP_REG);
    reg |= 0x01;
    return WriteRegister(SWITCH_OP_REG, reg);
}

bool KSZ9893RDriver::LinkStatus(uint8_t port) {
    uint8_t status = ReadRegister(PHY_STATUS_BASE + port);
    return (status & LINK_UP_BIT) != 0;
}

