/*
 * MCP3561_Driver.cpp
 *
 *  Created on: Feb 19, 2025
 *      Author: Local user
 */

#include "MCP3561_Driver.hpp"

MCPADCDriver::MCPADCDriver(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_gpio_,
                           uint16_t cs_pin_, uint8_t address)
    : hspi(hspi), cs_gpio(cs_gpio_), cs_pin(cs_pin_), address(address & 0b11) {}

bool MCPADCDriver::SetRegister8(REGISTER_t reg, uint8_t val) {
  // CMD[7:6] = part address
  // CMD[5:2] = register address
  // CMD[1:0] = cmd type, 0b10 for incremental write
  uint8_t cmdByte = (address << 6) | ((reg & 0b1111) << 2) | 0b10;
  uint8_t data[2] = {cmdByte, val};

  return SendSPI(data, sizeof(data));
}

bool MCPADCDriver::SetRegister16(REGISTER_t reg, uint16_t val) {
  uint8_t cmdByte = (address << 6) | ((reg & 0b1111) << 2) | 0b10;
  uint8_t data[3] = {cmdByte, static_cast<uint8_t>((val >> 8) & 0xff),
                     static_cast<uint8_t>(val & 0xff)};

  return SendSPI(data, sizeof(data));
}

bool MCPADCDriver::SetRegister24(REGISTER_t reg, uint32_t val) {
  uint8_t cmdByte = (address << 6) | ((reg & 0b1111) << 2) | 0b10;
  uint8_t data[4] = {cmdByte, static_cast<uint8_t>((val >> 16) & 0xff),
                     static_cast<uint8_t>((val >> 8) & 0xff),
                     static_cast<uint8_t>(val & 0xff)};

  return SendSPI(data, sizeof(data));
}

void MCPADCDriver::CSLow() {
  HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_RESET);
}

void MCPADCDriver::CSHigh() {
  HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_SET);
}

uint8_t MCPADCDriver::GetRegister8(REGISTER_t reg) {
  uint8_t cmdByte = (address << 6) | ((reg & 0b1111) << 2) | 0b01;
  uint8_t data[2] = {cmdByte};
  uint8_t received[2];
  SendReceiveSPI(data, sizeof(received), received);
  return received[1];
}

uint16_t MCPADCDriver::GetRegister16(REGISTER_t reg) {
  uint8_t cmdByte = (address << 6) | ((reg & 0b1111) << 2) | 0b01;
  uint8_t data[3] = {cmdByte};
  uint8_t received[3];
  SendReceiveSPI(data, sizeof(received), received);
  return (received[1] << 8) | received[2];
}

uint32_t MCPADCDriver::GetRegister24(REGISTER_t reg) {
  uint8_t cmdByte = (address << 6) | ((reg & 0b1111) << 2) | 0b01;
  uint8_t data[4] = {cmdByte};
  uint8_t received[4];
  SendReceiveSPI(data, sizeof(received), received);
  return (received[1] << 16) | (received[2] << 8) | received[3];
}

bool MCPADCDriver::SetField(const FieldInfo field, uint32_t val) {
  assert(field.writeable == true);
  if (field == OUTPUT_MODE.Info()) {
    outputModeCache = static_cast<OUTPUT_MODE_t::V>(val);
  }
  uint32_t reg = GetRegister(field.reg);
  uint32_t mask = ((((uint32_t)1) << field.GetNumBits()) - 1) << field.lsbIndex;
  return SetRegister(field.reg, (reg & ~mask) | (val << (field.lsbIndex)));
}

uint32_t MCPADCDriver::GetField(FieldInfo field) {
  uint32_t reg = GetRegister(field.reg);
  uint32_t mask = ((((uint32_t)1) << field.GetNumBits()) - 1) << field.lsbIndex;
  return (reg & mask) >> field.lsbIndex;
}

uint32_t MCPADCDriver::ReadADC() { return GetRegister(ADCDATA); }

uint32_t MCPADCDriver::GetRegister32(REGISTER_t reg) {
  uint8_t cmdByte = (address << 6) | ((reg & 0b1111) << 2) | 0b01;
  uint8_t data[5] = {cmdByte};
  uint8_t received[5];
  SendReceiveSPI(data, sizeof(received), received);
  return (received[1] << 24) | (received[2] << 16) | (received[3] << 8) |
         received[4];
}

bool MCPADCDriver::SendSPI(const uint8_t *data, uint16_t length) {
  CSLow();
  HAL_StatusTypeDef r = HAL_SPI_Transmit(hspi, data, length, 1000);
  CSHigh();
  return r == HAL_OK;
}

bool MCPADCDriver::SendReceiveSPI(const uint8_t *data, uint16_t length,
                                  uint8_t *output) {
  CSLow();
  HAL_StatusTypeDef r =
      HAL_SPI_TransmitReceive(hspi, data, output, length, 1000);
  CSHigh();
  return r == HAL_OK;
}

bool MCPADCDriver::IsRegisterReserved(REGISTER_t reg) {
  if (reg == 0xb || reg == 0xc || reg == 0xe) {
    return true;
  }
  return false;
}

bool MCPADCDriver::SetRegister(REGISTER_t reg, uint32_t val) {
  assert(IsRegisterReserved(reg) == false);
  uint8_t numBits = GetNumRegBits(reg);
  if (numBits == 8) {
    assert(val <= 0xff);
    return SetRegister8(reg, val);
  }
  if (numBits == 16) {
    assert(val <= 0xffff);
    return SetRegister16(reg, val);
  }
  if (numBits == 24) {
    assert(val <= 0xffffff);
    return SetRegister24(reg, val);
  }
  return false;
}

uint32_t MCPADCDriver::GetRegister(REGISTER_t reg) {
  uint8_t numBits = GetNumRegBits(reg);
  if (numBits == 8) {
    return GetRegister8(reg);
  }
  if (numBits == 16) {
    return GetRegister16(reg);
  }
  if (numBits == 24) {
    return GetRegister24(reg);
  }
  if (numBits == 32) {
    return GetRegister32(reg);
  }
  return false;
}

uint8_t MCPADCDriver::GetNumRegBits(REGISTER_t reg) {
  const static uint8_t bitTable[] = {0,  8,  8,  8,  8, 8, 8,  24,
                                     24, 24, 24, 24, 8, 8, 16, 16};
  if (reg == ADCDATA) {
    switch (outputModeCache) {
      case OUTPUT_MODE_t::V::OM_24_BIT:
        return 24;
      case OUTPUT_MODE_t::V::OM_32_BIT_0_PADDED:
      case OUTPUT_MODE_t::V::OM_32_BIT_CH_ID:
      case OUTPUT_MODE_t::V::OM_32_BIT_EXTENDED_SIGN:
        return 32;
      default:
        return 0;
    }
  }
  return bitTable[reg];
}

uint8_t MCPADCField::GetNumBits() { return Info().GetNumBits(); }

bool MCPADCField::operator==(const MCPADCField &other) {
  return this->getReg() == other.getReg() &&
         this->getLsbIndex() == other.getLsbIndex() &&
         this->getMsbIndex() == other.getMsbIndex();
}

const FieldInfo MCPADCField::Info() const {
  return {getReg(), getMsbIndex(), getLsbIndex(), writeable()};
}
