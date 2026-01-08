#ifndef KSZ9893R_DRIVER_HPP_
#define KSZ9893R_DRIVER_HPP_

#include "stm32l4xx_hal.h"
#include <stdint.h>

namespace KSZ9893R_REGISTER {
    enum REGISTER : uint16_t {
        CHIP_ID1 = 0x0001,
        CHIP_ID2 = 0x0002
    };
}
//chip IDs
#define KSZ9893R_CHIP_ID1_VAL 0x98
#define KSZ9893R_CHIP_ID2_VAL 0x93

class KSZ9893RDriver {
public:
        KSZ9893RDriver(SPI_HandleTypeDef* hspi,
            GPIO_TypeDef* cs_gpio,
            uint16_t cs_pin)
        : hspi(hspi), cs_gpio(cs_gpio), cs_pin(cs_pin) {}

    uint8_t ReadReg(uint16_t reg);
    void WriteReg(uint16_t reg, uint8_t val);

private:
    SPI_HandleTypeDef* hspi;
    GPIO_TypeDef* cs_gpio;
    uint16_t cs_pin;

    void CSLow() { HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_RESET); }
    void CSHigh() { HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_SET); }
    
    coid SPI_TxRx(uint8_t* tx, uint8_t* rx, uint16_t len) {
        HAL_SPI_TransmitReceive(hspi, tx, rx, len, HAL_MAX_DELAY);
    }
};

#endif