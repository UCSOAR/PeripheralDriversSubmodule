#ifndef KSZ9893R_DRIVER_HPP_
#define KSZ9893R_DRIVER_HPP_

#include "stm32l4xx_hal.h"
#include <stdint.h>

namespace KSZ9893R_REG {

    enum REG : uint16_t {
        CHIP_ID1        = 0x0000,
        CHIP_ID2        = 0x0001,
        SWITCH_OP       = 0x0300,
        PORT1_STATUS_1  = 0x011E,
        PORT2_STATUS_1  = 0x021E,
        PORT3_STATUS_1  = 0x031E,
        PORT1_CTRL_1    = 0x0104,
        PORT2_CTRL_1    = 0x0204,
        PORT3_CTRL_1    = 0x0304,
        PORT3_XMII_CTRL = 0x0312,
        PORT3_CLK_CTRL  = 0x0313
    };
}

class KSZ9893RDriver {
public:
    bool Init(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpio, uint16_t cs_pin);

    uint8_t ReadReg(KSZ9893R_REG::REG reg);
    bool    WriteReg(KSZ9893R_REG::REG reg, uint8_t value);
    bool ReadRegs(uint16_t startAddr, uint8_t* buffer, uint16_t length);
    bool SoftReset();
    bool LinkUp(uint8_t port);

private:
    SPI_HandleTypeDef* hspi = nullptr;
    GPIO_TypeDef* cs_gpio = nullptr;
    uint16_t cs_pin = 0;

    void CSLow();
    void CSHigh();

    bool SPI_Read(uint16_t addr, uint8_t* data, uint16_t len);
    bool SPI_Write(uint16_t addr, const uint8_t* data, uint16_t len);

    bool ConfigureRMII_Port3();
};

#endif
