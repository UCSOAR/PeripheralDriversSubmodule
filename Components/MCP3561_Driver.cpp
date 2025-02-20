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

bool MCPADCDriver::SetRegister8(uint8_t reg, uint8_t val) {
  // CMD[7:6] = part address
  // CMD[5:2] = register address
  // CMD[1:0] = cmd type, 0b10 for incremental write
  uint8_t cmdByte = (address << 6) | ((reg & 0b1111) << 2) | 0b10;
  uint8_t data[2] = {cmdByte, val};

  return SendSPI(data, sizeof(data));
}

bool MCPADCDriver::SetRegister16(uint8_t reg, uint16_t val) {
  uint8_t cmdByte = (address << 6) | ((reg & 0b1111) << 2) | 0b10;
  uint8_t data[3] = {cmdByte, (val >> 8) & 0xff, val & 0xff};

  return SendSPI(data, sizeof(data));
}

bool MCPADCDriver::SetRegister24(uint8_t reg, uint32_t val) {
  uint8_t cmdByte = (address << 6) | ((reg & 0b1111) << 2) | 0b10;
  uint8_t data[4] = {cmdByte, (val >> 16) & 0xff, (val >> 8) & 0xff,
                     val & 0xff};

  return SendSPI(data, sizeof(data));
}

void MCPADCDriver::CSLow() {
  HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_RESET);
}

void MCPADCDriver::CSHigh() {
  HAL_GPIO_WritePin(cs_gpio, cs_pin, GPIO_PIN_SET);
}

uint8_t MCPADCDriver::GetRegister8(uint8_t reg) {
  uint8_t cmdByte = (address << 6) | ((reg & 0b1111) << 2) | 0b01;
  uint8_t data[2] = {cmdByte};
  uint8_t received[2];
  SendReceiveSPI(data, sizeof(received), received);
  return received[1];
}

uint16_t MCPADCDriver::GetRegister16(uint8_t reg) {
  uint8_t cmdByte = (address << 6) | ((reg & 0b1111) << 2) | 0b01;
  uint8_t data[3] = {cmdByte};
  uint8_t received[3];
  SendReceiveSPI(data, sizeof(received), received);
  return (received[1] << 8) | received[2];
}

uint32_t MCPADCDriver::GetRegister24(uint8_t reg) {
  uint8_t cmdByte = (address << 6) | ((reg & 0b1111) << 2) | 0b01;
  uint8_t data[4] = {cmdByte};
  uint8_t received[4];
  SendReceiveSPI(data, sizeof(received), received);
  return (received[1] << 16) | (received[2] << 8) | received[3];
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
