/**
 ******************************************************************************
 * @file    MAX31856_regs.hpp
 * @brief   Register map and bit definitions for MAX31856.
 ******************************************************************************
 */

#ifndef MAX31856_REGS_HPP_
#define MAX31856_REGS_HPP_

#include <stdint.h>

namespace MAX31856_REG {

// Register addresses.
enum REG : uint8_t {
    CR0 = 0x00,
    CR1 = 0x01,
    MASK = 0x02,
    CJHF = 0x03,
    CJLF = 0x04,
    LTHFTH = 0x05,
    LTHFTL = 0x06,
    LTLFTH = 0x07,
    LTLFTL = 0x08,
    CJTO = 0x09,
    CJTH = 0x0A,
    CJTL = 0x0B,
    LTCBH = 0x0C,
    LTCBM = 0x0D,
    LTCBL = 0x0E,
    SR = 0x0F
};

// CR0 bit masks.
constexpr uint8_t CR0_CONV_MODE = 0x80; // 1 = automatic, 0 = normally off
constexpr uint8_t CR0_1SHOT = 0x40; // 1 = trigger one-shot conversion
constexpr uint8_t CR0_OCFAULT = 0x30; // Fault mode bits [5:4]
constexpr uint8_t CR0_CJ = 0x08; // 1 = enable cold-junction sensor
constexpr uint8_t CR0_FAULT = 0x04; // 1 = fault detection enable
constexpr uint8_t CR0_FAULTCLR = 0x02; // 1 = clear fault status
constexpr uint8_t CR0_50HZ = 0x01; // 1 = 50Hz, 0 = 60Hz

// CR1 bit masks and field values.
constexpr uint8_t CR1_AVG_MASK = 0x70; // Averaging [6:4]
constexpr uint8_t CR1_TYPE_MASK = 0x0F; // Thermocouple type [3:0]

constexpr uint8_t CR1_AVG_1 = 0x00;
constexpr uint8_t CR1_AVG_2 = 0x10;
constexpr uint8_t CR1_AVG_4 = 0x20;
constexpr uint8_t CR1_AVG_8 = 0x30;
constexpr uint8_t CR1_AVG_16 = 0x40;

constexpr uint8_t CR1_TYPE_B = 0x00;
constexpr uint8_t CR1_TYPE_E = 0x01;
constexpr uint8_t CR1_TYPE_J = 0x02;
constexpr uint8_t CR1_TYPE_K = 0x03;
constexpr uint8_t CR1_TYPE_N = 0x04;
constexpr uint8_t CR1_TYPE_R = 0x05;
constexpr uint8_t CR1_TYPE_S = 0x06;
constexpr uint8_t CR1_TYPE_T = 0x07;

// MASK register bits (1 = mask fault).
constexpr uint8_t MASK_CJHF = 0x01; // Cold-junction high
constexpr uint8_t MASK_CJLF = 0x02; // Cold-junction low
constexpr uint8_t MASK_TCHF = 0x04; // Thermocouple high
constexpr uint8_t MASK_TCLF = 0x08; // Thermocouple low
constexpr uint8_t MASK_OVUV = 0x10; // Over/under voltage
constexpr uint8_t MASK_OPEN = 0x20; // Open-circuit

// SR (fault status) bits.
constexpr uint8_t SR_CJHF = 0x01;
constexpr uint8_t SR_CJLF = 0x02;
constexpr uint8_t SR_TCHF = 0x04;
constexpr uint8_t SR_TCLF = 0x08;
constexpr uint8_t SR_OVUV = 0x10;
constexpr uint8_t SR_OPEN = 0x20;
constexpr uint8_t SR_RANGE = 0x40; // Linearized temperature out of range
constexpr uint8_t SR_CJ_RANGE = 0x80; // Cold-junction temperature out of range

} // namespace MAX31856_REG

#endif // MAX31856_REGS_HPP_
