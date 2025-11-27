#ifndef MAX31856_DRIVER_HPP_
#define MAX31856_DRIVER_HPP_

#include "stm32l4xx_hal.h"
#include <stdint.h>

namespace MAX31856_REG {
    enum REG : uint8_t {
        CR0 = 0x00,
        CR1 = 0x01,
        MASK = 0x02,
        CJHF = 0x03,
        CJLF = 0x04,
        LTHFTH = 0x05,
        LTHFTL = 0x06,
        LTLFTH = 0x07,
        LTLFTL = 0x08,
        CJTO = 0x09,
        CJTH = 0x0A,
        CJTL = 0x0B,
        LTCBH = 0x0C,
        LTCBM = 0x0D,
        LTCBL = 0x0E,
        SR = 0x0F
    };
}

class MAX31856Driver {
public:
    MAX31856Driver();
    ~MAX31856Driver();

    void Init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *gpio, uint16_t pin);

    bool SetRegister(MAX31856_REG::REG reg, uint8_t val);
    uint8_t GetRegister(MAX31856_REG::REG reg);
    void GetMultipleRegisters(MAX31856_REG::REG startreg, int numBytes, uint8_t *out);

    float ReadThermocoupleTempC();
    uint8_t GetFaultStatus();

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

