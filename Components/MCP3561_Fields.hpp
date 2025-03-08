/*
 * MCP3561_Fields.hpp
 *
 *  Created on: Feb 22, 2025
 *      Author: Local user
 */

#ifndef MCP3561_FIELDS_HPP_
#define MCP3561_FIELDS_HPP_

#define MCP3561_FIELD_NOT_WRITEABLE \
  bool writeable() { return false; }
#define MCP3561_FIELD_LSB_INDEX(i) \
  const BIT_INDEX getLsbIndex() const { return i; }
#define MCP3561_FIELD_MSB_INDEX(i) \
  const BIT_INDEX getMsbIndex() const { return i; }
#define MCP3561_FIELD_REGISTER(r) \
  const MCP3561_REGISTER_t getReg() const { return r; }

enum class MCP3561_REGISTER_t {
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
  _RESERVED1,
  _RESERVED2,
  LOCK,
  _RESERVED3,
  CRCCFG,
  _INVALID

};

typedef uint8_t BIT_INDEX;

class MCPADCDriver;

struct MCP3561_FieldInfo {
  MCP3561_REGISTER_t reg;
  BIT_INDEX msbIndex;
  BIT_INDEX lsbIndex;
  const uint8_t GetNumBits() const;
  const bool writeable;
  bool operator==(const MCP3561_FieldInfo &other) const;
};

// Base class for a bit field within an MCP3561 register.
// Derived classes just overload the virtual functions to return their own
// constants.
class MCPADCField {
 public:
  uint8_t GetNumBits();
  bool operator==(const MCPADCField &other);
  const MCP3561_FieldInfo Info() const;
  virtual const BIT_INDEX getLsbIndex() const { return 0; }
  virtual const BIT_INDEX getMsbIndex() const { return 0; }
  virtual const MCP3561_REGISTER_t getReg() const {
    return MCP3561_REGISTER_t::CONFIG0;
  }
  operator MCP3561_FieldInfo() const { return Info(); }

  virtual bool writeable() const { return true; }
};
namespace MCP3561Fields {
class VOLT_REF_t : public MCPADCField {
 public:
  enum V { VREF_INTERNAL = 1, VREF_EXTERNAL = 0 };

  MCP3561_FIELD_LSB_INDEX(7);
  MCP3561_FIELD_MSB_INDEX(7);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG0);
};

class CLOCK_t : public MCPADCField {
 public:
  enum V {
    CLK_INT_OUT = 0b11,
    CLK_INT_NO_OUT = 0b10,
    CLK_EXT_DIG = 0b01,
    CLK_EXT_DIG_DEF = 0b00
  };

  MCP3561_FIELD_LSB_INDEX(4);
  MCP3561_FIELD_MSB_INDEX(5);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG0);
};

class CS_t : public MCPADCField {
 public:
  enum V { CS_15uA = 0b11, CS_3u7A = 0b10, CS_0u9A = 0b01, CS_NONE = 0b00 };

  MCP3561_FIELD_LSB_INDEX(2);
  MCP3561_FIELD_MSB_INDEX(3);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG0);
};

class ADC_MODE_t : public MCPADCField {
 public:
  enum V {
    ADC_CONV = 0b11,
    ADC_STANDBY = 0b10,
    ADC_SHUTDOWN = 0b01,
    ADC_SHUTDOWN_DEF = 0b00
  };

  MCP3561_FIELD_LSB_INDEX(0);
  MCP3561_FIELD_MSB_INDEX(1);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG0);
};

class PRESCALAR_t : public MCPADCField {
 public:
  enum V { SCALAR_8 = 0b11, SCALAR_4 = 0b10, SCALAR_2 = 0b01, SCALAR_1 = 0b00 };

  MCP3561_FIELD_LSB_INDEX(6);
  MCP3561_FIELD_MSB_INDEX(7);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG1);
};

class OSR_t : public MCPADCField {
 public:
  MCP3561_FIELD_LSB_INDEX(2);
  MCP3561_FIELD_MSB_INDEX(5);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG1);
};

class CURRENTBOOST_t : public MCPADCField {
 public:
  enum V {
    BOOST_2X = 0b11,
    BOOST_1X = 0b10,
    BOOST_0X66 = 0b01,
    BOOST_0X5 = 0b00
  };

  MCP3561_FIELD_LSB_INDEX(6);
  MCP3561_FIELD_MSB_INDEX(7);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG2);
};

class GAIN_t : public MCPADCField {
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

  MCP3561_FIELD_LSB_INDEX(3);
  MCP3561_FIELD_MSB_INDEX(5);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG2);
};

class AUTOZEROMUX_t : public MCPADCField {
 public:
  MCP3561_FIELD_LSB_INDEX(2);
  MCP3561_FIELD_MSB_INDEX(2);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG2);
};

class AUTOZEROREF_t : public MCPADCField {
 public:
  MCP3561_FIELD_LSB_INDEX(1);
  MCP3561_FIELD_MSB_INDEX(1);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG2);
};

class CONV_MODE_t : public MCPADCField {
 public:
  enum V {
    ONE_SHOT_THEN_SHUTDOWN,
    ONE_SHOT_THEN_SHUTDOWN_,
    ONE_SHOT_THEN_STANDBY,
    CONTINUOUS
  };

  MCP3561_FIELD_LSB_INDEX(6);
  MCP3561_FIELD_MSB_INDEX(7);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG3);
};

class OUTPUT_MODE_t : public MCPADCField {
 public:
  enum V {
    OM_24_BIT,
    OM_32_BIT_0_PADDED,
    OM_32_BIT_EXTENDED_SIGN,
    OM_32_BIT_CH_ID
  };

  MCP3561_FIELD_LSB_INDEX(4);
  MCP3561_FIELD_MSB_INDEX(5);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG3);
};

class CRC_FORMAT_t : public MCPADCField {
 public:
  enum V { CRC_16, CRC_32 };

  MCP3561_FIELD_LSB_INDEX(3);
  MCP3561_FIELD_MSB_INDEX(3);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG3);
};

class CRC_READ_EN_t : public MCPADCField {
 public:
  MCP3561_FIELD_LSB_INDEX(2);
  MCP3561_FIELD_MSB_INDEX(2);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG3);
};

class OFFCAL_EN_t : public MCPADCField {
 public:
  MCP3561_FIELD_LSB_INDEX(1);
  MCP3561_FIELD_MSB_INDEX(1);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG3);
};

class GAINCAL_EN_t : public MCPADCField {
 public:
  MCP3561_FIELD_LSB_INDEX(0);
  MCP3561_FIELD_MSB_INDEX(0);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CONFIG3);
};

class DATAREADY_IRQ_t : public MCPADCField {
 public:
  MCP3561_FIELD_NOT_WRITEABLE;

  enum V { NEWREADY, NOTUPDATED };

  MCP3561_FIELD_LSB_INDEX(6);
  MCP3561_FIELD_MSB_INDEX(6);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::IRQ);
};

class CRCERR_IRQ_t : public MCPADCField {
 public:
  MCP3561_FIELD_NOT_WRITEABLE;
  enum V { ERROR, NOERROR };

  MCP3561_FIELD_LSB_INDEX(5);
  MCP3561_FIELD_MSB_INDEX(5);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::IRQ);
};

class POR_IRQ_t : public MCPADCField {
 public:
  MCP3561_FIELD_NOT_WRITEABLE;
  enum V { OCCURRED, NOTOCCURRED };

  MCP3561_FIELD_LSB_INDEX(4);
  MCP3561_FIELD_MSB_INDEX(4);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::IRQ);
};

class IRQ_MODE_t : public MCPADCField {
 public:
  enum V { ALLINTS_HIGHZ, ALLINTS_PULLUP, MDAT_HIGHZ, MDAT_PULLUP };

  MCP3561_FIELD_LSB_INDEX(2);
  MCP3561_FIELD_MSB_INDEX(3);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::IRQ);
};

class FAST_CMD_EN_t : public MCPADCField {
 public:
  MCP3561_FIELD_LSB_INDEX(1);
  MCP3561_FIELD_MSB_INDEX(1);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::IRQ);
};

class CONV_INT_EN_t : public MCPADCField {
 public:
  MCP3561_FIELD_LSB_INDEX(0);
  MCP3561_FIELD_MSB_INDEX(0);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::IRQ);
};

// Used for MUX_VIN_POS and MUX_VIN_NEG
enum class VREF {
  CH0,
  CH1,
  CH2,
  CH3,
  CH4,
  CH5,
  CH6,
  CH7,
  AGND,
  AVDD,
  _RESERVED,
  REFIN_POS,
  REFIN_NEG,
  TEMP_DIODE_P,
  TEMP_DIODE_M,
  VCM
};

class MUX_VIN_POS : public MCPADCField {
 public:
  MCP3561_FIELD_LSB_INDEX(4);
  MCP3561_FIELD_MSB_INDEX(7);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::MUX);
};

class MUX_VIN_NEG : public MCPADCField {
 public:
  MCP3561_FIELD_LSB_INDEX(0);
  MCP3561_FIELD_MSB_INDEX(3);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::MUX);
};

// If 0, no delay
// Otherwise, delay equal to 4*2^(value) multiples of DMCLK
class SCAN_DELAY : public MCPADCField {
 public:
  MCP3561_FIELD_LSB_INDEX(21);
  MCP3561_FIELD_MSB_INDEX(23);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::SCAN);
};

class SCAN_CHANNEL : public MCPADCField {
 public:
  enum V {
    CH0,
    CH1,
    CH2,
    CH3,
    CH4,
    CH5,
    CH6,
    CH7,
    DIFFA,
    DIFFB,
    DIFFC,
    DIFFD,
    TEMP,
    AVDD,
    VCM,
    OFFSET
  };

  MCP3561_FIELD_LSB_INDEX(0);
  MCP3561_FIELD_MSB_INDEX(15);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::SCAN);
};

class SCAN_TIMER : public MCPADCField {
 public:
  MCP3561_FIELD_LSB_INDEX(0);
  MCP3561_FIELD_MSB_INDEX(23);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::TIMER);
};

class OFFSETCALCODE : public MCPADCField {
 public:
  MCP3561_FIELD_LSB_INDEX(0);
  MCP3561_FIELD_MSB_INDEX(23);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::OFFSETCAL);
};

class GAINCALCODE : public MCPADCField {
 public:
  MCP3561_FIELD_LSB_INDEX(0);
  MCP3561_FIELD_MSB_INDEX(23);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::GAINCAL);
};

class SPI_LOCK : public MCPADCField {
 public:
  static constexpr uint8_t UNLOCK_CODE = 0xa5;

  MCP3561_FIELD_LSB_INDEX(0);
  MCP3561_FIELD_MSB_INDEX(7);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::LOCK);
};

class CRC_CHECKSUM : public MCPADCField {
 public:
  MCP3561_FIELD_NOT_WRITEABLE;

  MCP3561_FIELD_LSB_INDEX(0);
  MCP3561_FIELD_MSB_INDEX(15);
  MCP3561_FIELD_REGISTER(MCP3561_REGISTER_t::CRCCFG);
};

}  // namespace MCP3561Fields

inline const uint8_t MCP3561_FieldInfo::GetNumBits() const {
  return msbIndex - lsbIndex + 1;
}

inline bool MCP3561_FieldInfo::operator==(
    const MCP3561_FieldInfo &other) const {
  if (other.lsbIndex == lsbIndex && other.msbIndex == msbIndex &&
      other.reg == reg) {
    return true;
  }
  return false;
}

#endif /* MCP3561_FIELDS_HPP_ */
