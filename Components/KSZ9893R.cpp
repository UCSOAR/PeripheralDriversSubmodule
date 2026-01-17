#include "KSZ9893RDriver.hpp"
#include "stm32l4xx_hal.h"
#include <assert.h>

#define SPI_DUMMY_BYTE 0xFF
#define KSZ_SPI_READ   0x80
#define KSZ_SPI_WRITE  0x00


KSZ9893RDriver::KSZ9893RDriver() = default;
KSZ9893RDriver::~KSZ9893RDriver() = default;

void KSZ9893RDriver::Init(SPI_HandleTypeDef* hspi_,
                          GPIO_TypeDef* cs_gpio_,
                          uint16_t cs_pin_) {
    hspi = hspi_;
    initialized = true;

    SetCSPin(cs_gpio_, cs_pin_);
    CSHigh();

    // Verify SPI communication
    uint8_t id1 = GetRegister(KSZ9893R_REG::CHIP_ID1);
    uint8_t id2 = GetRegister(KSZ9893R_REG::CHIP_ID2);

    if(id1 != KSZ9893R_CHIP_ID1_VAL ||
       id2 != KSZ9893R_CHIP_ID2_VAL) {
        initialized = false;
    }
}

uint8_t KSZ9893RDriver::GetRegister(KSZ9893R_REG::REG reg) {
    assert(initialized);

    uint16_t addr = static_cast<uint16_t>(reg);

    uint8_t tx[4] = {
        uint8_t(KSZ_SPI_READ | ((addr >> 9) & 0x7F)),
        uint8_t((addr >> 1) & 0xFF),
        uint8_t((addr & 0x01) << 7),
        SPI_DUMMY_BYTE
    };

    uint8_t rx[4] = {0};

    CSLow();
    HAL_SPI_TransmitReceive(hspi, tx, rx, 4, 1000);
    CSHigh();

    return rx[3];
}

bool KSZ9893RDriver::SetRegister(KSZ9893R_REG::REG reg, uint8_t val) {
    assert(initialized);

    uint16_t addr = static_cast<uint16_t>(reg);

    uint8_t tx[4] = {
        uint8_t(KSZ_SPI_WRITE | ((addr >> 9) & 0x7F)),
        uint8_t((addr >> 1) & 0xFF),
        uint8_t((addr & 0x01) << 7),
        val
    };

    CSLow();
    HAL_StatusTypeDef r = HAL_SPI_Transmit(hspi, tx, 4, 1000);
    CSHigh();

    return r == HAL_OK;
}

void KSZ9893RDriver::GetMultipleRegisters(KSZ9893R_REG::REG startreg,
                                          int numBytes,
                                          uint8_t* out) {
    assert(initialized);

    for(int i = 0; i < numBytes; i++) {
        out[i] = GetRegister(
            KSZ9893R_REG::REG(uint16_t(startreg) + i)
        );
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
    uint8_t val = GetRegister(KSZ9893R_REG::SWITCH_OP);
    val |= 0x01;   // Soft reset bit
    return SetRegister(KSZ9893R_REG::SWITCH_OP, val);
}

bool KSZ9893RDriver::LinkStatus(uint8_t port) {
    static const KSZ9893R_REG::REG phy_status_regs[] = {
        KSZ9893R_REG::PHY1_STATUS,
        KSZ9893R_REG::PHY2_STATUS,
        KSZ9893R_REG::PHY3_STATUS
    };

    if(port < 1 || port > 3)
        return false;

    uint8_t status = GetRegister(phy_status_regs[port - 1]);

    // Bit 5 = Link Status (datasheet correct)
    return (status & 0x20) != 0;
}
