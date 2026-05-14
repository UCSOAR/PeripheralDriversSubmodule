/**************************************************************************/
/**
  @file     NAU7802.cpp
  @date 	2026-05-13
  @author 	Javier
*/
/**************************************************************************/
#include <NAU7802.h>

namespace {
constexpr uint32_t NAU7802_I2C_TIMEOUT_MS = 100;
}

/**************************************************************************/
/*!
    @brief  Instantiates a new NAU7802 class
*/
/**************************************************************************/
Adafruit_NAU7802::Adafruit_NAU7802(I2C_HandleTypeDef *hi2c,
                   uint8_t i2cAddress)
  : _hi2c(hi2c), _i2cAddress(i2cAddress) {}

/**************************************************************************/
/*!
    @brief  Sets up the I2C connection and tests that the sensor was found.
    @param hi2c Pointer to the STM32 HAL I2C handle to communicate with.
    @return true if sensor was found, otherwise false.
*/
/**************************************************************************/
bool Adafruit_NAU7802::begin(I2C_HandleTypeDef *hi2c,
                             uint8_t i2cAddress) {
  if (hi2c == nullptr) {
    return false;
  }

  _hi2c = hi2c;
  _i2cAddress = i2cAddress;

  if (HAL_I2C_IsDeviceReady(_hi2c, _i2cAddress << 1, 2,
                            NAU7802_I2C_TIMEOUT_MS) != HAL_OK) {
    return false;
  }

  if (!reset())
    return false;
  if (!enable(true))
    return false;

  /* Check for NAU7802 revision register (0x1F), low nibble should be 0xF. */
  uint8_t revision = 0;
  if (!readRegister(NAU7802_REVISION_ID, &revision)) {
    return false;
  }
  if ((revision & 0xF) != 0xF) {
    return false;
  }

  if (!setLDO(NAU7802_3V0))
    return false;
  if (!setGain(NAU7802_GAIN_128))
    return false;
  if (!setRate(NAU7802_RATE_10SPS))
    return false;

  // disable ADC chopper clock
  if (!writeBits(NAU7802_ADC, 0x30, 4, 0x3)) {
    return false;
  }

  // use low ESR caps
  if (!writeBits(NAU7802_PGA, 0x40, 6, 0)) {
    return false;
  }

  return true;
}

/**************************************************************************/
/*!
    @brief  Whether to have the sensor enabled and working or in power down mode
    @param  flag True to be in powered mode, False for power down mode
    @return False if something went wrong with I2C comms
*/
/**************************************************************************/
bool Adafruit_NAU7802::enable(bool flag) {
  if (!flag) {
    // shut down;
    if (!writeBits(NAU7802_PU_CTRL, 0x04, 2, 0))
      return false;
    if (!writeBits(NAU7802_PU_CTRL, 0x02, 1, 0))
      return false;
    return true;
  }
  // turn on!
  if (!writeBits(NAU7802_PU_CTRL, 0x02, 1, 1))
    return false;
  if (!writeBits(NAU7802_PU_CTRL, 0x04, 2, 1))
    return false;
  // RDY: Analog part wakeup stable plus Data Ready after exiting power-down
  // mode 600ms
  HAL_Delay(600);
  if (!writeBits(NAU7802_PU_CTRL, 0x10, 4, 1))
    return false;
  uint8_t ready = 0;
  if (!readBits(NAU7802_PU_CTRL, 0x08, 3, &ready)) {
    return false;
  }
  return (ready != 0);
}

/**************************************************************************/
/*!
    @brief Whether there is new ADC data to read
    @return True when there's new data available
*/
/**************************************************************************/
bool Adafruit_NAU7802::available(void) {
  uint8_t ready = 0;
  if (!readBits(NAU7802_PU_CTRL, 0x20, 5, &ready)) {
    return false;
  }
  return (ready != 0);
}

/**************************************************************************/
/*!
    @brief  Set which channel for ADC
    @param channel Set to 0 for CH1, 1 for CH2
    @returns False if any I2C error occured
*/
/**************************************************************************/
bool Adafruit_NAU7802::setChannel(uint8_t channel) {
  if (channel > 1)
    channel = 1;
  return writeBits(NAU7802_CTRL2, 0x80, 7, channel);
}

/**************************************************************************/
/*!
    @brief Read the stored 24-bit ADC output value.
    @return Signed integer with ADC output result, extended to a int32_t
*/
/**************************************************************************/
int32_t Adafruit_NAU7802::read(void) {
  uint8_t buffer[3] = {0, 0, 0};
  if (!readRegister(NAU7802_ADCO_B2, buffer, sizeof(buffer))) {
    return 0;
  }
  uint32_t val = (static_cast<uint32_t>(buffer[0]) << 16) |
                 (static_cast<uint32_t>(buffer[1]) << 8) |
                 static_cast<uint32_t>(buffer[2]);
  // extend sign bit
  if (val & 0x800000) {
    val |= 0xFF000000;
  }

  return val;
}

/**************************************************************************/
/*!
    @brief Perform a soft reset
    @return False if there was any I2C comms error
*/
/**************************************************************************/
bool Adafruit_NAU7802::reset(void) {
  // Set the RR bit to 1 in R0x00, to guarantee a reset of all register values.
  if (!writeBits(NAU7802_PU_CTRL, 0x01, 0, 1))
    return false;
  HAL_Delay(10);
  // Set the RR bit to 0 and PUD bit 1, in R0x00, to enter normal operation
  if (!writeBits(NAU7802_PU_CTRL, 0x01, 0, 0))
    return false;
  if (!writeBits(NAU7802_PU_CTRL, 0x02, 1, 1))
    return false;
  // After about 200 microseconds, the PWRUP bit will be Logic=1 indicating the
  // device is ready for the remaining programming setup.
  HAL_Delay(1);
  uint8_t ready = 0;
  if (!readBits(NAU7802_PU_CTRL, 0x08, 3, &ready)) {
    return false;
  }
  return (ready != 0);
}

/**************************************************************************/
/*!
    @brief  The desired LDO voltage setter
    @param voltage The LDO setting: NAU7802_4V5, NAU7802_4V2, NAU7802_3V9,
    NAU7802_3V6, NAU7802_3V3, NAU7802_3V0, NAU7802_2V7, NAU7802_2V4, or
    NAU7802_EXTERNAL if we are not using the internal LDO
    @return False if there was any I2C comms error
*/
/**************************************************************************/
bool Adafruit_NAU7802::setLDO(NAU7802_LDOVoltage voltage) {
  if (voltage == NAU7802_EXTERNAL) {
    // special case!
    return writeBits(NAU7802_PU_CTRL, 0x80, 7, 0);
  }

  // internal LDO
  if (!writeBits(NAU7802_PU_CTRL, 0x80, 7, 1))
    return false;
  return writeBits(NAU7802_CTRL1, 0x38, 3, static_cast<uint8_t>(voltage));
}

/**************************************************************************/
/*!
    @brief  The desired LDO voltage getter
    @returns The voltage setting: NAU7802_4V5, NAU7802_4V2, NAU7802_3V9,
    NAU7802_3V6, NAU7802_3V3, NAU7802_3V0, NAU7802_2V7, NAU7802_2V4, or
    NAU7802_EXTERNAL if we are not using the internal LDO
*/
/**************************************************************************/
NAU7802_LDOVoltage Adafruit_NAU7802::getLDO(void) {
  uint8_t avdds = 0;
  if (!readBits(NAU7802_PU_CTRL, 0x80, 7, &avdds)) {
    return NAU7802_EXTERNAL;
  }
  if (!avdds) {
    return NAU7802_EXTERNAL;
  }
  // internal LDO
  uint8_t vldo = 0;
  if (!readBits(NAU7802_CTRL1, 0x38, 3, &vldo)) {
    return NAU7802_EXTERNAL;
  }
  return static_cast<NAU7802_LDOVoltage>(vldo);
}

/**************************************************************************/
/*!
    @brief  The desired ADC gain setter
    @param  gain Desired gain: NAU7802_GAIN_1, NAU7802_GAIN_2, NAU7802_GAIN_4,
    NAU7802_GAIN_8, NAU7802_GAIN_16, NAU7802_GAIN_32, NAU7802_GAIN_64,
    or NAU7802_GAIN_128
    @returns False if there was any error during I2C comms
*/
/**************************************************************************/
bool Adafruit_NAU7802::setGain(NAU7802_Gain gain) {
  return writeBits(NAU7802_CTRL1, 0x07, 0, static_cast<uint8_t>(gain));
}

/**************************************************************************/
/*!
    @brief  The desired ADC gain getter
    @returns The gain: NAU7802_GAIN_1, NAU7802_GAIN_2, NAU7802_GAIN_4,
    NAU7802_GAIN_8, NAU7802_GAIN_16, NAU7802_GAIN_32, NAU7802_GAIN_64,
    or NAU7802_GAIN_128
*/
/**************************************************************************/
NAU7802_Gain Adafruit_NAU7802::getGain(void) {
  uint8_t gain = 0;
  if (!readBits(NAU7802_CTRL1, 0x07, 0, &gain)) {
    return NAU7802_GAIN_1;
  }
  return static_cast<NAU7802_Gain>(gain);
}

/**************************************************************************/
/*!
    @brief  The desired conversion rate setter
    @param rate The desired rate: NAU7802_RATE_10SPS, NAU7802_RATE_20SPS,
    NAU7802_RATE_40SPS, NAU7802_RATE_80SPS, or NAU7802_RATE_320SPS
    @returns False if any I2C error occured
*/
/**************************************************************************/
bool Adafruit_NAU7802::setRate(NAU7802_SampleRate rate) {
  return writeBits(NAU7802_CTRL2, 0x70, 4, static_cast<uint8_t>(rate));
}

/**************************************************************************/
/*!
    @brief  The desired conversion rate getter
    @returns The rate: NAU7802_RATE_10SPS, NAU7802_RATE_20SPS,
    NAU7802_RATE_40SPS, NAU7802_RATE_80SPS, or NAU7802_RATE_320SPS
*/
/**************************************************************************/
NAU7802_SampleRate Adafruit_NAU7802::getRate(void) {
  uint8_t rate = 0;
  if (!readBits(NAU7802_CTRL2, 0x70, 4, &rate)) {
    return NAU7802_RATE_10SPS;
  }
  return static_cast<NAU7802_SampleRate>(rate);
}

/**************************************************************************/
/*!
    @brief  Enable or disable optional PGA filters. NOTE - this should only
    be used for single channel operation.
    @param enable Use true to enable or false to disable.
    @returns False if any I2C error occured
*/
/**************************************************************************/
bool Adafruit_NAU7802::setPGACap(bool enable) {
  return writeBits(NAU7802_POWER, 0x80, 7, enable ? 1 : 0);
}

/**************************************************************************/
/*!
    @brief Enable or disable optional PGA bypass.
    @param enable Use true to enable or false to disable
    @return False if any I2C error occurred
*/
/**************************************************************************/
bool Adafruit_NAU7802::setPGABypass(bool enable) {
  return writeBits(NAU7802_PGA, 0x10, 4, enable ? 1 : 0);
}

/**************************************************************************/
/*!
    @brief  Perform the internal calibration procedure
    @param mode The calibration mode to perform: NAU7802_CALMOD_INTERNAL,
    NAU7802_CALMOD_OFFSET or NAU7802_CALMOD_GAIN
    @returns True on calibrations success
*/
/**************************************************************************/
bool Adafruit_NAU7802::calibrate(NAU7802_Calibration mode) {
  if (!writeBits(NAU7802_CTRL2, 0x03, 0, static_cast<uint8_t>(mode)))
    return false;
  if (!writeBits(NAU7802_CTRL2, 0x04, 2, 1))
    return false;
  while (true) {
    uint8_t started = 0;
    if (!readBits(NAU7802_CTRL2, 0x04, 2, &started)) {
      return false;
    }
    if (started) {
      break;
    }
    HAL_Delay(10);
  }

  uint8_t error = 0;
  if (!readBits(NAU7802_CTRL2, 0x08, 3, &error)) {
    return false;
  }
  return (error == 0);
}

bool Adafruit_NAU7802::writeRegister(uint8_t reg, uint8_t value) {
  if (_hi2c == nullptr) {
    return false;
  }
  return (HAL_I2C_Mem_Write(_hi2c, _i2cAddress << 1, reg, I2C_MEMADD_SIZE_8BIT,
                            &value, 1, NAU7802_I2C_TIMEOUT_MS) == HAL_OK);
}

bool Adafruit_NAU7802::readRegister(uint8_t reg, uint8_t *value) {
  if (_hi2c == nullptr || value == nullptr) {
    return false;
  }
  return (HAL_I2C_Mem_Read(_hi2c, _i2cAddress << 1, reg, I2C_MEMADD_SIZE_8BIT,
                           value, 1, NAU7802_I2C_TIMEOUT_MS) == HAL_OK);
}

bool Adafruit_NAU7802::readRegister(uint8_t reg, uint8_t *buffer, size_t len) {
  if (_hi2c == nullptr || buffer == nullptr || len == 0) {
    return false;
  }
  return (HAL_I2C_Mem_Read(_hi2c, _i2cAddress << 1, reg, I2C_MEMADD_SIZE_8BIT,
                           buffer, static_cast<uint16_t>(len),
                           NAU7802_I2C_TIMEOUT_MS) == HAL_OK);
}

bool Adafruit_NAU7802::writeBits(uint8_t reg, uint8_t mask, uint8_t shift,
                                 uint8_t value) {
  uint8_t regValue = 0;
  if (!readRegister(reg, &regValue)) {
    return false;
  }
  regValue = static_cast<uint8_t>((regValue & ~mask) | ((value << shift) & mask));
  return writeRegister(reg, regValue);
}

bool Adafruit_NAU7802::readBits(uint8_t reg, uint8_t mask, uint8_t shift,
                                uint8_t *value) {
  uint8_t regValue = 0;
  if (!readRegister(reg, &regValue)) {
    return false;
  }
  if (value != nullptr) {
    *value = static_cast<uint8_t>((regValue & mask) >> shift);
  }
  return true;
}
