#include "KSZ9893RDriver.hpp"
#include "stm32l4xx_hal.h"

#define KSZ_SPI_READ   0x80
#define KSZ_SPI_WRITE  0x00
#define SPI_DUMMY_BYTE 0xFF

#define KSZ9893R_CHIP_ID1_VAL 0x98
#define KSZ9893R_CHIP_ID2_VAL 0x93


void KSZ9893RDriver::Init(SPI_HandleTypeDef* hspi_, GPIO_TypeDef* cs_gpio_, uint16_t cs_pin_) {

    hspi = hspi_;
    SetCSPin(cs_gpio_, cs_pin_);
    initialized = true;

    CSHigh();

    uint8_t id1 = GetRegister(KSZ9893R_REG::CHIP_ID1);
    uint8_t id2 = GetRegister(KSZ9893R_REG::CHIP_ID2);

    if (id1 != KSZ9893R_CHIP_ID1_VAL ||
        id2 != KSZ9893R_CHIP_ID2_VAL) {
        initialized = false;
    }
}

uint8_t KSZ9893RDriver::GetRegister(KSZ9893R_REG::REG reg) {
    if (!initialized) return 0;

    uint16_t addr = static_cast<uint16_t>(reg);

    uint8_t tx[4];
    uint8_t rx[4] = {0};

    tx[0] = KSZ_SPI_READ | ((addr >> 8) & 0x7F);
    tx[1] = addr & 0xFF;
    tx[2] = SPI_DUMMY_BYTE;
    tx[3] = SPI_DUMMY_BYTE;

    CSLow();
    HAL_SPI_TransmitReceive(hspi, tx, rx, 4, HAL_MAX_DELAY);
    CSHigh();

    return rx[3];
}

bool KSZ9893RDriver::SetRegister(KSZ9893R_REG::REG reg, uint8_t val) {
    if (!initialized) return false;

    uint16_t addr = static_cast<uint16_t>(reg);

    uint8_t tx[4];
    tx[0] = KSZ_SPI_WRITE | ((addr >> 8) & 0x7F);
    tx[1] = addr & 0xFF;
    tx[2] = val;
    tx[3] = SPI_DUMMY_BYTE;

    CSLow();
    HAL_StatusTypeDef r = HAL_SPI_Transmit(hspi, tx, 4, HAL_MAX_DELAY);
    CSHigh();

    return r == HAL_OK;
}

void KSZ9893RDriver::GetMultipleRegisters(KSZ9893R_REG::REG startReg, uint16_t numBytes, uint8_t* out) {
    if (!initialized || out == nullptr || numBytes == 0) return;

    uint16_t addr = static_cast<uint16_t>(startReg);

    uint8_t header[2];
    header[0] = KSZ_SPI_READ | ((addr >> 8) & 0x7F);
    header[1] = addr & 0xFF;

    CSLow();
    HAL_SPI_Transmit(hspi, header, 2, HAL_MAX_DELAY);
    HAL_SPI_Receive(hspi, out, numBytes, HAL_MAX_DELAY);
    CSHigh();
}

bool KSZ9893RDriver::SoftReset() {
    uint8_t val = GetRegister(KSZ9893R_REG::SWITCH_OP);
    val |= 0x01;
    bool ok = SetRegister(KSZ9893R_REG::SWITCH_OP, val);
    HAL_Delay(100);
    return ok;
}

bool KSZ9893RDriver::LinkStatus(uint8_t port) {
    if (port < 1 || port > 3) return false;

    static const KSZ9893R_REG::REG portStatus[] = {
        KSZ9893R_REG::PORT1_STATUS_1,
        KSZ9893R_REG::PORT2_STATUS_1,
        KSZ9893R_REG::PORT3_STATUS_1
    };

    uint8_t status = GetRegister(portStatus[port - 1]);

    return (status & 0x01);
}

void KSZ9893RDriver::CSLow() {
    HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_RESET);
}

void KSZ9893RDriver::CSHigh() {
    HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_SET);
}

void KSZ9893RDriver::SetCSPin(GPIO_TypeDef* gpio, uint16_t pin) {
    cs_gpio = gpio;
    cs_pin = pin;
}

