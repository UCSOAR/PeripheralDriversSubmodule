#include "KSZ9893RDriver.hpp"
#include <string.h>


#define KSZ_SPI_READ   0x80
#define KSZ_SPI_WRITE  0x00
#define KSZ_SPI_AI     0x40

#define KSZ9893R_CHIP_ID1_VAL 0x98
#define KSZ9893R_CHIP_ID2_VAL 0x93

void KSZ9893RDriver::CSLow() {
    HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_RESET);
}

void KSZ9893RDriver::CSHigh() {
    HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_SET);
}


bool KSZ9893RDriver::SPI_Read(uint16_t addr, uint8_t* data, uint16_t len)
{
    if (!hspi || !data || len == 0)
        return false;

    uint8_t header[3];
    uint8_t cmd = KSZ_SPI_READ;

    if (len > 1)
        cmd |= KSZ_SPI_AI;

    header[0] = cmd | ((addr >> 10) & 0x3F);
    header[1] = (addr >> 2) & 0xFF;
    header[2] = (addr & 0x03) << 6;

    CSLow();

    if (HAL_SPI_Transmit(hspi, header, 3, HAL_MAX_DELAY) != HAL_OK ||
        HAL_SPI_Receive(hspi, data, len, HAL_MAX_DELAY) != HAL_OK)
    {
        CSHigh();
        return false;
    }

    CSHigh();
    return true;
}

bool KSZ9893RDriver::SPI_Write(uint16_t addr, const uint8_t* data, uint16_t len)
{
    if (!hspi || !data || len == 0)
        return false;

    uint8_t header[3];
    uint8_t cmd = KSZ_SPI_WRITE;

    if (len > 1)
        cmd |= KSZ_SPI_AI;

    header[0] = cmd | ((addr >> 10) & 0x3F);
    header[1] = (addr >> 2) & 0xFF;
    header[2] = (addr & 0x03) << 6;

    CSLow();

    if (HAL_SPI_Transmit(hspi, header, 3, HAL_MAX_DELAY) != HAL_OK ||
        HAL_SPI_Transmit(hspi, (uint8_t*)data, len, HAL_MAX_DELAY) != HAL_OK)
    {
        CSHigh();
        return false;
    }

    CSHigh();
    return true;
}

uint8_t KSZ9893RDriver::ReadReg(KSZ9893R_REG::REG reg)
{
    uint8_t val = 0;
    SPI_Read(static_cast<uint16_t>(reg), &val, 1);
    return val;
}

bool KSZ9893RDriver::WriteReg(KSZ9893R_REG::REG reg, uint8_t value)
{
    return SPI_Write(static_cast<uint16_t>(reg), &value, 1);
}

bool KSZ9893RDriver::ReadRegs(uint16_t startAddr, uint8_t* buffer, uint16_t length)
{
    return SPI_Read(startAddr, buffer, length);
}

bool KSZ9893RDriver::SoftReset()
{
    WriteReg(KSZ9893R_REG::SWITCH_OP, 0x01);

    while (ReadReg(KSZ9893R_REG::SWITCH_OP) & 0x01) {
        HAL_Delay(1);
    }

    HAL_Delay(20);
    return true;
}

bool KSZ9893RDriver::ConfigureRMII_Port3()
{
    WriteReg(KSZ9893R_REG::PORT3_XMII_CTRL, 0x02);

    WriteReg(KSZ9893R_REG::PORT3_CLK_CTRL, 0x03);

    uint8_t ctrl = ReadReg(KSZ9893R_REG::PORT3_CTRL_1);
    WriteReg(KSZ9893R_REG::PORT3_CTRL_1, ctrl | (1 << 7));

    HAL_Delay(10);
    return true;
}

bool KSZ9893RDriver::Init(SPI_HandleTypeDef* hspi_, GPIO_TypeDef* cs_gpio_, uint16_t cs_pin_)
{
    hspi = hspi_;
    cs_gpio = cs_gpio_;
    cs_pin = cs_pin_;

    CSHigh();
    HAL_Delay(20);

    uint8_t id1 = ReadReg(KSZ9893R_REG::CHIP_ID1);
    uint8_t id2 = ReadReg(KSZ9893R_REG::CHIP_ID2);

    if (id1 != KSZ9893R_CHIP_ID1_VAL ||
        id2 != KSZ9893R_CHIP_ID2_VAL)
        return false;

    SoftReset();

    WriteReg(KSZ9893R_REG::PORT1_CTRL_1,
             ReadReg(KSZ9893R_REG::PORT1_CTRL_1) & ~0x01);
    WriteReg(KSZ9893R_REG::PORT2_CTRL_1,
             ReadReg(KSZ9893R_REG::PORT2_CTRL_1) & ~0x01);
    WriteReg(KSZ9893R_REG::PORT3_CTRL_1,
             ReadReg(KSZ9893R_REG::PORT3_CTRL_1) & ~0x01);

    ConfigureRMII_Port3();
    return true;
}

bool KSZ9893RDriver::LinkUp(uint8_t port)
{
    KSZ9893R_REG::REG reg;

    switch (port) {
        case 1: reg = KSZ9893R_REG::PORT1_STATUS_1; break;
        case 2: reg = KSZ9893R_REG::PORT2_STATUS_1; break;
        case 3: reg = KSZ9893R_REG::PORT3_STATUS_1; break;
        default: return false;
    }


    return (ReadReg(reg) & 0x01) != 0;
}
