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
  uint8_t data[3] = {cmdByte, (uint8_t)((val >> 8) & 0xff), val & 0xff};

  return SendSPI(data, sizeof(data));
}

bool MCPADCDriver::SetRegister24(REGISTER_t reg, uint32_t val) {
  uint8_t cmdByte = (address << 6) | ((reg & 0b1111) << 2) | 0b10;
  uint8_t data[4] = {cmdByte, (uint8_t)((val >> 16) & 0xff),
                     (uint8_t)((val >> 8) & 0xff), val & 0xff};

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

bool MCPADCDriver::SetVoltageReference(VOLT_REF ref) {
  uint8_t c0 = GetRegister(CONFIG0);
  return SetRegister(CONFIG0, (c0 & 0b01111111) | (ref << 7));
}

MCPADCDriver::VOLT_REF MCPADCDriver::GetVoltageReference() {
  return (VOLT_REF)((GetRegister(CONFIG0) >> 7) & 0b1);
}

bool MCPADCDriver::SetClk(CLOCK clk) {
  uint8_t c0 = GetRegister(CONFIG0);
  return SetRegister(CONFIG0, (c0 & 0b11001111) | (clk << 4));
}

MCPADCDriver::CLOCK MCPADCDriver::GetClk() {
  return (CLOCK)((GetRegister(CONFIG0) >> 4) & 0b11);
}

bool MCPADCDriver::SetCurrentSource(CS cs) {
  uint8_t c0 = GetRegister(CONFIG0);
  return SetRegister(CONFIG0, (c0 & 0b11110011) | (cs << 2));
}

MCPADCDriver::CS MCPADCDriver::GetCurrentSource() {
  return (CS)((GetRegister(CONFIG0) >> 2) & 0b11);
}

bool MCPADCDriver::SetADCMode(ADC_MODE mode) {
  uint8_t c0 = GetRegister(CONFIG0);
  return SetRegister(CONFIG0, (c0 & 0b11111100) | (mode));
}

MCPADCDriver::ADC_MODE MCPADCDriver::GetADCMode() {
  return (ADC_MODE)((GetRegister(CONFIG0)) & 0b11);
}

bool MCPADCDriver::SetPrescalar(PRESCALAR prescalar) {
  uint8_t c1 = GetRegister(CONFIG1);
  return SetRegister(CONFIG1, (c1 & 0b00111111) | (prescalar << 6));
}

MCPADCDriver::PRESCALAR MCPADCDriver::GetPrescalar() {
  return (PRESCALAR)((GetRegister(CONFIG1) >> 6) & 0b11);
}

bool MCPADCDriver::SetOSR(uint8_t osrbits) {
  uint8_t c1 = GetRegister(CONFIG1);
  return SetRegister(CONFIG1, (c1 & 0b11000011) | (osrbits << 2));
}

uint8_t MCPADCDriver::GetOSR() {
  return ((GetRegister(CONFIG1) >> 2) & 0b1111);
}

bool MCPADCDriver::SetCurrentBoost(CURRENTBOOST boost) {
  uint8_t c2 = GetRegister(CONFIG2);
  return SetRegister(CONFIG2, (c2 & 0b00111111) | (boost << 6));
}

MCPADCDriver::CURRENTBOOST MCPADCDriver::GetCurrentBoost() {
  return (CURRENTBOOST)((GetRegister(CONFIG2) >> 6) & 0b11);
}

bool MCPADCDriver::SetGain(GAIN gain) {
  uint8_t c2 = GetRegister(CONFIG2);
  return SetRegister(CONFIG2, (c2 & 0b11000111) | (gain << 3));
}

MCPADCDriver::GAIN MCPADCDriver::GetGain() {
  return (GAIN)((GetRegister(CONFIG2) >> 3) & 0b111);
}

bool MCPADCDriver::SetAutoZero(bool enabled) {
  uint8_t c2 = GetRegister(CONFIG2);
  return SetRegister(CONFIG2, (c2 & 0b11111011) | (enabled << 2));
}

bool MCPADCDriver::GetAutoZero() {
  return (GAIN)((GetRegister(CONFIG2) >> 2) & 0b1);
}

bool MCPADCDriver::SetAutoRef(bool enabled) {
  uint8_t c2 = GetRegister(CONFIG2);
  return SetRegister(CONFIG2, (c2 & 0b11111101) | (enabled << 1));
}

bool MCPADCDriver::GetAutoRef() {
  return (GAIN)((GetRegister(CONFIG2) >> 1) & 0b1);
}

bool MCPADCDriver::SetConvMode(CONV_MODE mode) {
  uint8_t c3 = GetRegister(CONFIG3);
  return SetRegister(CONFIG3, (c3 & 0b00111111) | (mode << 6));
}

MCPADCDriver::CONV_MODE MCPADCDriver::GetConvMode() {
  return (CONV_MODE)((GetRegister(CONFIG3) >> 6) & 0b11);
}

bool MCPADCDriver::SetOutputMode(OUTPUT_MODE_t mode) {
  uint8_t c3 = GetRegister(CONFIG3);
  return SetRegister(CONFIG3, (c3 & 0b11001111) | (mode << 4));
}

MCPADCDriver::OUTPUT_MODE_t MCPADCDriver::GetOutputMode() {
  return (OUTPUT_MODE_t)((GetRegister(CONFIG3) >> 4) & 0b11);
}

bool MCPADCDriver::SetCRC(CRC_FORMAT format) {
  uint8_t c3 = GetRegister(CONFIG3);
  return SetRegister(CONFIG3, (c3 & 0b11110111) | (format << 3));
}

MCPADCDriver::CRC_FORMAT MCPADCDriver::GetCRC() {
  return (CRC_FORMAT)((GetRegister(CONFIG3) >> 3) & 0b1);
}

bool MCPADCDriver::SetCRCRead(bool enabled) {
  uint8_t c3 = GetRegister(CONFIG3);
  return SetRegister(CONFIG3, (c3 & 0b11111011) | (enabled << 2));
}

bool MCPADCDriver::GetCRCRead() { return ((GetRegister(CONFIG3) >> 2) & 0b1); }

bool MCPADCDriver::SetOffCal(bool enabled) {
  uint8_t c3 = GetRegister(CONFIG3);
  return SetRegister(CONFIG3, (c3 & 0b11111101) | (enabled << 1));
}

bool MCPADCDriver::GetOffCal() { return ((GetRegister(CONFIG3) >> 1) & 0b1); }

bool MCPADCDriver::SetGainCal(bool enabled) {
  uint8_t c3 = GetRegister(CONFIG3);
  return SetRegister(CONFIG3, (c3 & 0b11111110) | (enabled));
}

bool MCPADCDriver::GetGainCal() { return ((GetRegister(CONFIG3)) & 0b1); }

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
  if (reg == 0) {
    switch (outputModeCache) {
      case OM_24_BIT:
        return 24;
      case OM_32_BIT_0_PADDED:
      case OM_32_BIT_CH_ID:
      case OM_32_BIT_EXTENDED_SIGN:
        return 32;
      default:
        return 0;
    }
  }
  return bitTable[reg];
}
