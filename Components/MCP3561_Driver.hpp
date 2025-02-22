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
  enum REGISTER_t {
    ADCDATA = 0x0,
    CONFIG0,
    CONFIG1,
    CONFIG2,
    CONFIG3,
    IRQ,
    MUX,
    SCAN,
    TIMER,
    OFFSETCAL,
    GAINCAL,
    _R1,
    _R2,
    LOCK,
    _R3,
    CRCCFG

  };

  MCPADCDriver(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpio_,
               uint16_t cs_pin_, uint8_t address);

  bool SetRegister(REGISTER_t reg, uint32_t val);
  uint32_t GetRegister(REGISTER_t reg);

  enum VOLT_REF { VREF_INTERNAL = 1, VREF_EXTERNAL = 0 };
  bool SetVoltageReference(VOLT_REF ref);
  VOLT_REF GetVoltageReference();

  enum CLOCK {
    CLK_INT_OUT = 0b11,
    CLK_INT_NO_OUT = 0b10,
    CLK_EXT_DIG = 0b01,
    CLK_EXT_DIG_DEF = 0b00
  };
  bool SetClk(CLOCK clk);
  CLOCK GetClk();

  enum CS { CS_15uA = 0b11, CS_3u7A = 0b10, CS_0u9A = 0b01, CS_NONE = 0b00 };
  bool SetCurrentSource(CS cs);
  CS GetCurrentSource();

  enum ADC_MODE {
    ADC_CONV = 0b11,
    ADC_STANDBY = 0b10,
    ADC_SHUTDOWN = 0b01,
    ADC_SHUTDOWN_DEF = 0b00
  };
  bool SetADCMode(ADC_MODE mode);
  ADC_MODE GetADCMode();

  enum PRESCALAR {
    SCALAR_8 = 0b11,
    SCALAR_4 = 0b10,
    SCALAR_2 = 0b01,
    SCALAR_1 = 0b00
  };
  bool SetPrescalar(PRESCALAR prescalar);
  PRESCALAR GetPrescalar();

  bool SetOSR(uint8_t osrbits);
  uint8_t GetOSR();

  enum CURRENTBOOST {
    BOOST_2X = 0b11,
    BOOST_1X = 0b10,
    BOOST_0X66 = 0b01,
    BOOST_0X5 = 0b00
  };
  bool SetCurrentBoost(CURRENTBOOST boost);
  CURRENTBOOST GetCurrentBoost();

  enum GAIN {
    GAIN_THIRD,
    GAIN_1,
    GAIN_2,
    GAIN_4,
    GAIN_8,
    GAIN_16,
    GAIN_32,
    GAIN_64
  };
  bool SetGain(GAIN gain);
  GAIN GetGain();

  bool SetAutoZero(bool enabled);
  bool GetAutoZero();
  bool SetAutoRef(bool enabled);
  bool GetAutoRef();

  enum CONV_MODE {
    ONE_SHOT_THEN_SHUTDOWN,
    ONE_SHOT_THEN_SHUTDOWN_,
    ONE_SHOT_THEN_STANDBY,
    CONTINUOUS
  };
  bool SetConvMode(CONV_MODE mode);
  CONV_MODE GetConvMode();

  enum OUTPUT_MODE_t {
    OM_24_BIT,
    OM_32_BIT_0_PADDED,
    OM_32_BIT_EXTENDED_SIGN,
    OM_32_BIT_CH_ID
  };
  bool SetOutputMode(OUTPUT_MODE_t mode);
  OUTPUT_MODE_t GetOutputMode();

  enum CRC_FORMAT { CRC_16, CRC_32 };
  bool SetCRC(CRC_FORMAT format);
  CRC_FORMAT GetCRC();

  bool SetCRCRead(bool enabled);
  bool GetCRCRead();
  bool SetOffCal(bool enabled);
  bool GetOffCal();
  bool SetGainCal(bool enabled);
  bool GetGainCal();

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
  bool IsRegisterReserved(REGISTER_t reg);

  uint8_t GetNumRegBits(REGISTER_t reg);

  bool SetRegister8(REGISTER_t reg, uint8_t val);
  bool SetRegister16(REGISTER_t reg, uint16_t val);
  bool SetRegister24(REGISTER_t reg, uint32_t val);
  uint8_t GetRegister8(REGISTER_t reg);
  uint16_t GetRegister16(REGISTER_t reg);
  uint32_t GetRegister24(REGISTER_t reg);
  uint32_t GetRegister32(REGISTER_t reg);

  OUTPUT_MODE_t outputModeCache;
};

#endif /* MCP3561_DRIVER_HPP_ */
