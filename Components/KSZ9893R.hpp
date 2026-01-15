#ifndef KSZ9893R_DRIVER_HPP_
#define KSZ9893R_DRIVER_HPP_

#include "stm32l4xx_hal.h"
#include <stdint.h>

namespace KSZ9893R_REG {
    enum REG : uint16_t {
        CHIP_ID1    = 0x0000,
        CHIP_ID2    = 0x0001,
        SWITCH_OP   = 0x0300,

        PHY1_STATUS = 0x0100,
        PHY2_STATUS = 0x0101,
        PHY3_STATUS = 0x0102
    };
}

#define KSZ9893R_CHIP_ID1_VAL 0x98
#define KSZ9893R_CHIP_ID2_VAL 0x93

class KSZ9893RDriver {
public:
    KSZ9893RDriver();
    ~KSZ9893RDriver();

    void Init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *gpio, uint16_t pin);

    bool SetRegister(KSZ9893R_REG::REG reg, uint8_t val);
    uint8_t GetRegister(KSZ9893R_REG::REG reg);
    void GetMultipleRegisters(KSZ9893R_REG::REG startreg,
                              int numBytes,
                              uint8_t *out);

    bool SoftReset();
    bool LinkStatus(uint8_t port);

private:
    SPI_HandleTypeDef *hspi = nullptr;
    GPIO_TypeDef *cs_gpio = nullptr;
    uint16_t cs_pin = 0;
    bool initialized = false;

    void CSLow();
    void CSHigh();
    void SetCSPin(GPIO_TypeDef *gpio, uint16_t pin);
};

#endif