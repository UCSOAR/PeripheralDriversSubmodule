#include "KSZ9893RDriver.hpp"

#define KSZ_SPI_READ   0x80
#define KSZ_SPI_WRITE  0x00
#define SPI_DUMMY      0xFF

#define KSZ9893R_CHIP_ID1_VAL 0x98
#define KSZ9893R_CHIP_ID2_VAL 0x93


void KSZ9893RDriver::CSLow() {
    HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_RESET);
}

void KSZ9893RDriver::CSHigh() {
    HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_SET);
}


bool KSZ9893RDriver::Init(SPI_HandleTypeDef* hspi_, GPIO_TypeDef* cs_gpio_, uint16_t cs_pin_) {

    hspi = hspi_;
    cs_gpio = cs_gpio_;
    cs_pin = cs_pin_;
    initialized = false;

    CSHigh();
    HAL_Delay(10);

    uint8_t id1 = GetRegister(KSZ9893R_REG::CHIP_ID1);
    uint8_t id2 = GetRegister(KSZ9893R_REG::CHIP_ID2);

    if (id1 == KSZ9893R_CHIP_ID1_VAL &&
        id2 == KSZ9893R_CHIP_ID2_VAL) {
        initialized = true;
        return true;
    }

    return false;
}


bool KSZ9893RDriver::SPI_Read(uint16_t addr, uint8_t* data, uint16_t len) {
    if (!initialized || !hspi || !data || len == 0)
        return false;

    uint8_t header[3];
    header[0] = KSZ_SPI_READ | ((addr >> 8) & 0x7F);
    header[1] = addr & 0xFF;
    header[2] = SPI_DUMMY;

    CSLow();

    if (HAL_SPI_Transmit(hspi, header, 3, HAL_MAX_DELAY) != HAL_OK) {
        CSHigh();
        return false;
    }

    if (HAL_SPI_Receive(hspi, data, len, HAL_MAX_DELAY) != HAL_OK) {
        CSHigh();
        return false;
    }

    CSHigh();
    return true;
}


bool KSZ9893RDriver::SPI_Write(uint16_t addr, const uint8_t* data, uint16_t len) {
    if (!initialized || !hspi || !data || len == 0)
        return false;

    uint8_t header[2];
    header[0] = KSZ_SPI_WRITE | ((addr >> 8) & 0x7F);
    header[1] = addr & 0xFF;

    CSLow();

    if (HAL_SPI_Transmit(hspi, header, 2, HAL_MAX_DELAY) != HAL_OK) {
        CSHigh();
        return false;
    }

    if (HAL_SPI_Transmit(hspi, (uint8_t*)data, len, HAL_MAX_DELAY) != HAL_OK) {
        CSHigh();
        return false;
    }

    CSHigh();
    return true;
}


uint8_t KSZ9893RDriver::GetRegister(KSZ9893R_REG::REG reg) {
    uint8_t val = 0;

    if (!SPI_Read(static_cast<uint16_t>(reg), &val, 1))
        return 0;

    return val;
}


bool KSZ9893RDriver::SetRegister(KSZ9893R_REG::REG reg, uint8_t value) {
    return SPI_Write(static_cast<uint16_t>(reg), &value, 1);
}


bool KSZ9893RDriver::ReadRegisters(KSZ9893R_REG::REG startReg, uint16_t length, uint8_t* buffer) {
    return SPI_Read(static_cast<uint16_t>(startReg), buffer, length);
}


bool KSZ9893RDriver::SoftReset() {
    uint8_t val = GetRegister(KSZ9893R_REG::SWITCH_OP);
    val |= 0x01;
    bool ok = SetRegister(KSZ9893R_REG::SWITCH_OP, val);
    HAL_Delay(100);
    return ok;
}


bool KSZ9893RDriver::LinkStatus(uint8_t port) {
    if (port < 1 || port > 3)
        return false;

    static const KSZ9893R_REG::REG statusRegs[3] = {
        KSZ9893R_REG::PORT1_STATUS_1,
        KSZ9893R_REG::PORT2_STATUS_1,
        KSZ9893R_REG::PORT3_STATUS_1
    };

    uint8_t status = GetRegister(statusRegs[port - 1]);


    return (status & 0x01) != 0;
}



