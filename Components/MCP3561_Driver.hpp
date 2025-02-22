/*
 * MCP3561_Driver.hpp
 *
 *  Created on: Feb 19, 2025
 *      Author: Local user
 */

#ifndef MCP3561_DRIVER_HPP_
#define MCP3561_DRIVER_HPP_

#include <stm32h7xx.h>

#include "MCP3561_Fields.hpp"

class MCPADCDriver {
 public:
  MCPADCDriver(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_gpio_,
               uint16_t cs_pin_, uint8_t address);

  bool SetRegister(REGISTER_t reg, uint32_t val);
  uint32_t GetRegister(REGISTER_t reg);

  bool SetField(MCPADCField field, uint32_t val) {
    return field.Set(val, *this);
  };
  uint32_t GetField(MCPADCField field) { return field.Get(*this); };

  ADC_MODE_t ADC_MODE;

 protected:
 private:
  SPI_HandleTypeDef *hspi;
  GPIO_TypeDef *cs_gpio;
  const uint16_t cs_pin;
  // The 2-bit hardcoded address of the part
  const uint8_t address;

  void CSLow();
  void CSHigh();

  bool SendSPI(const uint8_t *data, uint16_t length);
  bool SendReceiveSPI(const uint8_t *data, uint16_t length, uint8_t *output);
  bool IsRegisterReserved(REGISTER_t reg);

  uint8_t GetNumRegBits(REGISTER_t reg);

  bool SetRegister8(REGISTER_t reg, uint8_t val);
  bool SetRegister16(REGISTER_t reg, uint16_t val);
  bool SetRegister24(REGISTER_t reg, uint32_t val);
  uint8_t GetRegister8(REGISTER_t reg);
  uint16_t GetRegister16(REGISTER_t reg);
  uint32_t GetRegister24(REGISTER_t reg);
  uint32_t GetRegister32(REGISTER_t reg);

  OUTPUT_MODE_t::V outputModeCache;
};

#endif /* MCP3561_DRIVER_HPP_ */
