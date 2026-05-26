#ifndef MAX31856_DRIVER_HPP_
#define MAX31856_DRIVER_HPP_

#if __has_include("stm32g4xx_hal.h")
    #include "stm32g4xx_hal.h"
#elif __has_include("stm32h7xx_hal.h")
    #include "stm32h7xx_hal.h"
#else
    #error "STM HAL Header found"
#endif



#include "MAX31856_regs.hpp"
#include <stdint.h>

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

    bool SetMASK(uint8_t value);
    uint8_t GetMASK();
    bool SetCJHF(uint8_t value);
    uint8_t GetCJHF();
    bool SetCR0(uint8_t value);
    uint8_t GetCR0();
    bool SetCR1(uint8_t value);
    uint8_t GetCR1();
    bool SetCJLF(uint8_t value);
    uint8_t GetCJLF();
    bool SetLTHFTH(uint8_t value);
    uint8_t GetLTHFTH();
    bool SetLTHFTL(uint8_t value);
    uint8_t GetLTHFTL();
    bool SetLTLFTH(uint8_t value);
    uint8_t GetLTLFTH();
    bool SetCJTO(uint8_t value);
    uint8_t GetCJTO();
    bool SetCJTH(uint8_t value);
    uint8_t GetCJTH();
    bool SetCJTL(uint8_t value);
    uint8_t GetCJTL();
    bool SetLTLFTL(uint8_t value);
    uint8_t GetLTLFTL();

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
