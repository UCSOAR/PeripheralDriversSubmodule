#ifndef KSZ9893R_DRIVER_HPP_
#define KSZ9893R_DRIVER_HPP_

#include "stm32l4xx_hal.h"
#include <stdint.h>

namespace KSZ9893R_REG {
    enum REG : uint16_t {
        CHIP_ID1       = 0x0000,
        CHIP_ID2       = 0x0001,
        SWITCH_OP      = 0x0300,
        PORT1_STATUS_1 = 0x011E,
        PORT2_STATUS_1 = 0x021E,
        PORT3_STATUS_1 = 0x031E
    };
}

class KSZ9893RDriver {
public:
    bool Init(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpio, uint16_t cs_pin);

    uint8_t GetRegister(KSZ9893R_REG::REG reg);
    bool    SetRegister(KSZ9893R_REG::REG reg, uint8_t value);

    bool ReadRegisters(KSZ9893R_REG::REG startReg, uint16_t length, uint8_t* buffer);

    bool SoftReset();
    bool LinkStatus(uint8_t port);

private:
    SPI_HandleTypeDef* hspi = nullptr;
    GPIO_TypeDef* cs_gpio = nullptr;
    uint16_t cs_pin = 0;
    bool initialized = false;

    void CSLow();
    void CSHigh();

    bool SPI_Read(uint16_t addr, uint8_t* data, uint16_t len);
    bool SPI_Write(uint16_t addr, const uint8_t* data, uint16_t len);
};

#endif



