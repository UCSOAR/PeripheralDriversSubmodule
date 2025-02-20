/*
 * MCP3561_Driver.hpp
 *
 *  Created on: Feb 19, 2025
 *      Author: Local user
 */

#ifndef MCP3561_DRIVER_HPP_
#define MCP3561_DRIVER_HPP_

#include <stm32h7xx.h>

class MCPADCDriver {
 public:
  MCPADCDriver(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpio_,
               uint16_t cs_pin_, uint8_t address);

  bool SetRegister8(uint8_t reg, uint8_t val);
  bool SetRegister16(uint8_t reg, uint16_t val);
  bool SetRegister24(uint8_t reg, uint32_t val);
  uint8_t GetRegister8(uint8_t reg);
  uint16_t GetRegister16(uint8_t reg);
  uint32_t GetRegister24(uint8_t reg);

 protected:
 private:
  SPI_HandleTypeDef* hspi;
  GPIO_TypeDef* cs_gpio;
  const uint16_t cs_pin;
  // The 2-bit hardcoded address of the part
  const uint8_t address;

  void CSLow();
  void CSHigh();

  bool SendSPI(const uint8_t* data, uint16_t length);
  bool SendReceiveSPI(const uint8_t* data, uint16_t length, uint8_t* output);
};

#endif /* MCP3561_DRIVER_HPP_ */
