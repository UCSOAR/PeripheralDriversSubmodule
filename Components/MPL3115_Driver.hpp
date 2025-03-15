/*
 * MPL3115_Driver.hpp
 *
 *  Created on: Feb 19, 2025
 *      Author: Local user
 */

#ifndef MPL3115_DRIVER_HPP_
#define MPL3115_DRIVER_HPP_

#include <stm32h7xx.h>

#include "MPL3115_Fields.hpp"

class MPLSENSDriver {
 public:
  MPLSENSDriver(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_gpio_,
                uint16_t cs_pin_, uint8_t address);

  bool SetRegister(MPL3115_REGISTER_t reg, uint32_t val);
  uint32_t GetRegister(MPL3115_REGISTER_t reg);

  bool SetField(const MPL3115_FieldInfo field, uint32_t val);
  uint32_t GetField(MPL3115_FieldInfo field);

  uint32_t ReadADC();

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
  bool IsRegisterReserved(MPL3115_REGISTER_t reg);

  uint8_t GetNumRegBits(MPL3115_REGISTER_t reg);

  bool SetRegister8(MPL3115_REGISTER_t reg, uint8_t val);
  bool SetRegister16(MPL3115_REGISTER_t reg, uint16_t val);
  bool SetRegister24(MPL3115_REGISTER_t reg, uint32_t val);
  uint8_t GetRegister8(MPL3115_REGISTER_t reg);
  uint16_t GetRegister16(MPL3115_REGISTER_t reg);
  uint32_t GetRegister24(MPL3115_REGISTER_t reg);
  uint32_t GetRegister32(MPL3115_REGISTER_t reg);
};

#endif /* MPL3115_DRIVER_HPP_ */
