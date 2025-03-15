/*
 * MPL3115_Driver.cpp
 *
 *  Created on: Feb 19, 2025
 *      Author: Local user
 */

#include "MPL3115_Driver.hpp"
using namespace MPL3115Fields;

MPLSENSDriver::MPLSENSDriver(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_gpio_,
                             uint16_t cs_pin_, uint8_t address)
    : hspi(hspi), cs_gpio(cs_gpio_), cs_pin(cs_pin_), address(address & 0b11) {}

bool MPLSENSDriver::SetRegister8(MPL3115_REGISTER_t reg, uint8_t val) {
  // CMD[7:6] = part address
  // CMD[5:2] = register address
  // CMD[1:0] = cmd type, 0b10 for incremental write
  uint8_t cmdByte =
      (address << 6) | ((static_cast<int>(reg) & 0b1111) << 2) | 0b10;
  uint8_t data[2] = {cmdByte, val};

  return SendSPI(data, sizeof(data));
}

void MPLSENSDriver::CSLow() {
  HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_RESET);
}

void MPLSENSDriver::CSHigh() {
  HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_SET);
}

uint8_t MPLSENSDriver::GetRegister8(MPL3115_REGISTER_t reg) {
  uint8_t cmdByte =
      (address << 6) | ((static_cast<int>(reg) & 0b1111) << 2) | 0b01;
  uint8_t data[2] = {cmdByte};
  uint8_t received[2];
  SendReceiveSPI(data, sizeof(received), received);
  return received[1];
}

bool MPLSENSDriver::SetField(const MPL3115_FieldInfo field, uint32_t val) {
  assert(field.writeable == true);
  uint32_t reg = GetRegister(field.reg);
  uint32_t mask = ((((uint32_t)1) << field.GetNumBits()) - 1) << field.lsbIndex;
  return SetRegister(field.reg, (reg & ~mask) | (val << (field.lsbIndex)));
}

uint32_t MPLSENSDriver::GetField(MPL3115_FieldInfo field) {
  uint32_t reg = GetRegister(field.reg);
  uint32_t mask = ((((uint32_t)1) << field.GetNumBits()) - 1) << field.lsbIndex;
  return (reg & mask) >> field.lsbIndex;
}

bool MPLSENSDriver::SendSPI(const uint8_t *data, uint16_t length) {
  CSLow();
  HAL_StatusTypeDef r = HAL_SPI_Transmit(hspi, data, length, 1000);
  CSHigh();
  return r == HAL_OK;
}

bool MPLSENSDriver::SendReceiveSPI(const uint8_t *data, uint16_t length,
                                   uint8_t *output) {
  CSLow();
  HAL_StatusTypeDef r =
      HAL_SPI_TransmitReceive(hspi, data, output, length, 1000);
  CSHigh();
  return r == HAL_OK;
}

bool MPLSENSDriver::IsRegisterReserved(MPL3115_REGISTER_t reg) { return false; }

bool MPLSENSDriver::SetRegister(MPL3115_REGISTER_t reg, uint32_t val) {
  assert(IsRegisterReserved(reg) == false);
  assert(val <= 0xff);
  return SetRegister8(reg, val);
}

uint32_t MPLSENSDriver::GetRegister(MPL3115_REGISTER_t reg) {
  return GetRegister8(reg);
}

uint8_t MPLSENSField::GetNumBits() { return Info().GetNumBits(); }

bool MPLSENSField::operator==(const MPLSENSField &other) {
  return this->getReg() == other.getReg() &&
         this->getLsbIndex() == other.getLsbIndex() &&
         this->getMsbIndex() == other.getMsbIndex();
}

const MPL3115_FieldInfo MPLSENSField::Info() const {
  return {getReg(), getMsbIndex(), getLsbIndex(), writeable()};
}
