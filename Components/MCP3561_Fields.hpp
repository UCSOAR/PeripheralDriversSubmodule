/*
 * MCP3561_Fields.hpp
 *
 *  Created on: Feb 22, 2025
 *      Author: Local user
 */

#ifndef MCP3561_FIELDS_HPP_
#define MCP3561_FIELDS_HPP_

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
  CRCCFG,
  _INVALID

};

class MCPADCDriver;

class MCPADCField {
 public:
  const bool Set(uint32_t val, MCPADCDriver& driver);
  const uint32_t Get(MCPADCDriver& driver);
  uint8_t GetNumBits();

 protected:
  virtual const REGISTER_t getReg() { return _INVALID; };
  virtual const uint8_t getMsbIndex() { return 255; };
  virtual const uint8_t getLsbIndex() { return 255; };
};

class VOLT_REF_t : public MCPADCField {
 public:
  enum V { VREF_INTERNAL = 1, VREF_EXTERNAL = 0 };
  const uint8_t getLsbIndex() { return 7; }
  const uint8_t getMsbIndex() { return 7; }
  const REGISTER_t getReg() { return CONFIG0; }
};

class CLOCK_t : public MCPADCField {
 public:
  enum V {
    CLK_INT_OUT = 0b11,
    CLK_INT_NO_OUT = 0b10,
    CLK_EXT_DIG = 0b01,
    CLK_EXT_DIG_DEF = 0b00
  };
  const uint8_t getLsbIndex() { return 4; }
  const uint8_t getMsbIndex() { return 5; }
  const REGISTER_t getReg() { return CONFIG0; }
};

class CS_t : MCPADCField {
 public:
  enum V { CS_15uA = 0b11, CS_3u7A = 0b10, CS_0u9A = 0b01, CS_NONE = 0b00 };
  const uint8_t getLsbIndex() { return 2; }
  const uint8_t getMsbIndex() { return 3; }
  const REGISTER_t getReg() { return CONFIG0; }
};

class ADC_MODE_t : public MCPADCField {
 public:
  enum V {
    ADC_CONV = 0b11,
    ADC_STANDBY = 0b10,
    ADC_SHUTDOWN = 0b01,
    ADC_SHUTDOWN_DEF = 0b00
  };
  const uint8_t getLsbIndex() { return 0; }
  const uint8_t getMsbIndex() { return 1; }
  const REGISTER_t getReg() { return CONFIG0; }
};

class PRESCALAR_t : MCPADCField {
 public:
  enum V { SCALAR_8 = 0b11, SCALAR_4 = 0b10, SCALAR_2 = 0b01, SCALAR_1 = 0b00 };
  const uint8_t getLsbIndex() { return 6; }
  const uint8_t getMsbIndex() { return 7; }
  const REGISTER_t getReg() { return CONFIG1; }
};

class OSR_t : MCPADCField {
 public:
  const uint8_t getLsbIndex() { return 2; }
  const uint8_t getMsbIndex() { return 5; }
  const REGISTER_t getReg() { return CONFIG1; }
};

class CURRENTBOOST_t : MCPADCField {
 public:
  enum V {
    BOOST_2X = 0b11,
    BOOST_1X = 0b10,
    BOOST_0X66 = 0b01,
    BOOST_0X5 = 0b00
  };
  const uint8_t getLsbIndex() { return 6; }
  const uint8_t getMsbIndex() { return 7; }
  const REGISTER_t getReg() { return CONFIG2; }
};

class GAIN_t : MCPADCField {
 public:
  enum V {
    GAIN_THIRD,
    GAIN_1,
    GAIN_2,
    GAIN_4,
    GAIN_8,
    GAIN_16,
    GAIN_32,
    GAIN_64
  };
  const uint8_t getLsbIndex() { return 3; }
  const uint8_t getMsbIndex() { return 5; }
  const REGISTER_t getReg() { return CONFIG2; }
};

class AUTOZEROMUX_t : MCPADCField {
 public:
  const uint8_t getLsbIndex() { return 2; }
  const uint8_t getMsbIndex() { return 2; }
  const REGISTER_t getReg() { return CONFIG2; }
};

class AUTOZEROREF_t : MCPADCField {
 public:
  const uint8_t getLsbIndex() { return 1; }
  const uint8_t getMsbIndex() { return 1; }
  const REGISTER_t getReg() { return CONFIG2; }
};

class CONV_MODE_t : MCPADCField {
 public:
  enum V {
    ONE_SHOT_THEN_SHUTDOWN,
    ONE_SHOT_THEN_SHUTDOWN_,
    ONE_SHOT_THEN_STANDBY,
    CONTINUOUS
  };
  const uint8_t getLsbIndex() { return 6; }
  const uint8_t getMsbIndex() { return 7; }
  const REGISTER_t getReg() { return CONFIG3; }
};

class OUTPUT_MODE_t : MCPADCField {
 public:
  enum V {
    OM_24_BIT,
    OM_32_BIT_0_PADDED,
    OM_32_BIT_EXTENDED_SIGN,
    OM_32_BIT_CH_ID
  };
  const uint8_t getLsbIndex() { return 4; }
  const uint8_t getMsbIndex() { return 5; }
  const REGISTER_t getReg() { return CONFIG3; }
};

class CRC_FORMAT_t : MCPADCField {
 public:
  enum V { CRC_16, CRC_32 };
  const uint8_t getLsbIndex() { return 3; }
  const uint8_t getMsbIndex() { return 3; }
  const REGISTER_t getReg() { return CONFIG3; }
};

class CRC_READ_EN_t : MCPADCField {
 public:
  const uint8_t getLsbIndex() { return 2; }
  const uint8_t getMsbIndex() { return 2; }
  const REGISTER_t getReg() { return CONFIG3; }
};

class OFFCAL_EN_t : MCPADCField {
 public:
  const uint8_t getLsbIndex() { return 1; }
  const uint8_t getMsbIndex() { return 1; }
  const REGISTER_t getReg() { return CONFIG3; }
};

class GAINCAL_EN_t : MCPADCField {
 public:
  const uint8_t getLsbIndex() { return 0; }
  const uint8_t getMsbIndex() { return 0; }
  const REGISTER_t getReg() { return CONFIG3; }
};

#endif /* MCP3561_FIELDS_HPP_ */
