#include "KSZ9893RDriver.hpp"
#include <assert.h>


#define KSZ_SPI_READ   0x80
#define KSZ_SPI_WRITE  0x00
#define KSZ_SPI_AI     0x40

#define KSZ9893R_ID1   0x98
#define KSZ9893R_ID2   0x93


bool KSZ9893RDriver::Init(SPI_HandleTypeDef* hspi_, GPIO_TypeDef* cs_gpio_, uint16_t cs_pin_)
{
    hspi = hspi_;
    cs_gpio = cs_gpio_;
    cs_pin = cs_pin_;
    initialized = true;

    CSHigh();
    HAL_Delay(20);

    //read/verify registers
    uint8_t id1 = GetRegister(KSZ9893R::CHIP_ID1);
    uint8_t id2 = GetRegister(KSZ9893R::CHIP_ID2);

    if (id1 != KSZ9893R_ID1 || id2 != KSZ9893R_ID2)
    {
        initialized = false;
        return false;
    }

    //reset for clean state
    SoftReset();

    // Enable all ports
    SetRegister(KSZ9893R::PORT1_CTRL_1,
                GetRegister(KSZ9893R::PORT1_CTRL_1) & ~0x01);
    SetRegister(KSZ9893R::PORT2_CTRL_1,
                GetRegister(KSZ9893R::PORT2_CTRL_1) & ~0x01);
    SetRegister(KSZ9893R::PORT3_CTRL_1,
                GetRegister(KSZ9893R::PORT3_CTRL_1) & ~0x01);
//config port3 as RMII CPU port
    ConfigureRMII_Port3();
    return true;
}

bool KSZ9893RDriver::SetRegister(KSZ9893R::Register reg, uint8_t val)
{
    assert(initialized);
    return SpiWrite(static_cast<uint16_t>(reg), &val, 1);
}

uint8_t KSZ9893RDriver::GetRegister(KSZ9893R::Register reg)
{
    assert(initialized);
    uint8_t val = 0;
    SpiRead(static_cast<uint16_t>(reg), &val, 1);
    return val;
}

bool KSZ9893RDriver::IsLinkUp(uint8_t port)
{
    assert(initialized);
    KSZ9893R::Register reg;

    switch (port)
    {
        case 1: reg = KSZ9893R::PORT1_STATUS_1; break;
        case 2: reg = KSZ9893R::PORT2_STATUS_1; break;
        case 3: reg = KSZ9893R::PORT3_STATUS_1; break;
        default: return false;
    }

    return (GetRegister(reg) & 0x01) != 0;
}


void KSZ9893RDriver::SoftReset()
{
    //set software reset
    SetRegister(KSZ9893R::SWITCH_OP, 0x01);

    while (GetRegister(KSZ9893R::SWITCH_OP) & 0x01)
    {
        HAL_Delay(1);
    }

    HAL_Delay(20);
}


void KSZ9893RDriver::ConfigureRMII_Port3()
{
    //selects RMII mode
    SetRegister(KSZ9893R::PORT3_XMII_CTRL, 0x02);

    //Enable RMII refernce clock
    SetRegister(KSZ9893R::PORT3_CLK_CTRL, 0x03);

        //enable RMII transmit
    uint8_t ctrl = GetRegister(KSZ9893R::PORT3_CTRL_1);
    SetRegister(KSZ9893R::PORT3_CTRL_1, ctrl | (1 << 7));
}

void KSZ9893RDriver::CSLow()
{
    assert(initialized);
    HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_RESET);
}

void KSZ9893RDriver::CSHigh()
{
    assert(initialized);
    HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_SET);
}


bool KSZ9893RDriver::SpiRead(uint16_t address, uint8_t* buffer, uint16_t length)
{
    assert(initialized);

    uint8_t header[3];
    uint8_t cmd = KSZ_SPI_READ;

    if (length > 1)
        cmd |= KSZ_SPI_AI;

    header[0] = cmd | ((address >> 10) & 0x3F);
    header[1] = (address >> 2) & 0xFF;
    header[2] = (address & 0x03) << 6;

    CSLow();
    HAL_SPI_Transmit(hspi, header, 3, 1000);
    HAL_SPI_Receive(hspi, buffer, length, 1000);
    CSHigh();

    return true;
}

bool KSZ9893RDriver::SpiWrite(uint16_t address, const uint8_t* buffer, uint16_t length)
{
    assert(initialized);

    uint8_t header[3];
    uint8_t cmd = KSZ_SPI_WRITE;

    //enable auto-increment for multi-byte writes
    if (length > 1)
        cmd |= KSZ_SPI_AI;

    //build spi header
    header[0] = cmd | ((address >> 10) & 0x3F);
    header[1] = (address >> 2) & 0xFF;
    header[2] = (address & 0x03) << 6;

    CSLow();
    HAL_SPI_Transmit(hspi, header, 3, 1000);
    HAL_SPI_Transmit(hspi, (uint8_t*)buffer, length, 1000);
    CSHigh();

    return true;
}

