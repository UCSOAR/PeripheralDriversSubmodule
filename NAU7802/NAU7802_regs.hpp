 /**
 ********************************************************************************
 * @file    nau7802_regs.hpp
 * @author  Javier
 * @date    2025-11-25
 * @brief   Register definitions and constants for the NAU7802 ADC.
 ********************************************************************************
 */



/************************************
 * MACROS AND DEFINES
 ************************************/

#ifndef NAU7802_REGS_HPP
#define NAU7802_REGS_HPP

// Fixed 7-bit I2C address of the NAU7802. [left-shifted for HAL]
#define NAU7802_I2C_ADDRESS (0x2A << 1)

// Register Map
#define NAU7802_REG_PU_CTRL      0x00
#define NAU7802_REG_CTRL1        0x01
#define NAU7802_REG_CTRL2        0x02
#define NAU7802_REG_ADC_B2       0x12 // ADC Result MSB
#define NAU7802_REG_ADC_B1       0x13 // ADC Result Mid-byte
#define NAU7802_REG_ADC_B0       0x14 // ADC Result LSB
#define NAU7802_REG_REVISION_ID  0x1F

// Gain [in] settings for register
#define NAU7802_GAIN_1X          0b000
#define NAU7802_GAIN_2X          0b001
#define NAU7802_GAIN_4X          0b010
#define NAU7802_GAIN_8X          0b011
#define NAU7802_GAIN_16X         0b100
#define NAU7802_GAIN_32X         0b101
#define NAU7802_GAIN_64X         0b110
#define NAU7802_GAIN_128X        0b111

// PU_CTRL Register Bits
#define NAU7802_PU_CTRL_RR       (1 << 0) // Register Reset
#define NAU7802_PU_CTRL_PUD      (1 << 1) // Power Up Digital
#define NAU7802_PU_CTRL_PUA      (1 << 2) // Power Up Analog
#define NAU7802_PU_CTRL_PUR      (1 << 3) // Power Up Ready (Read Only)
#define NAU7802_PU_CTRL_CS       (1 << 4) // Cycle Start
#define NAU7802_PU_CTRL_CR       (1 << 5) // Cycle Ready (Read Only)
#define NAU7802_PU_CTRL_OSCS     (1 << 6) // Select clock source
#define NAU7802_PU_CTRL_AVDDS    (1 << 7) // Select internal LDO

// CTRL2 Register Bits
#define NAU7802_CTRL2_CHS        (1 << 7) // Channel Select
#define NAU7802_CTRL2_CRS_MASK   (0b111 << 4) // Conversion Rate Select Mask
#define NAU7802_CTRL2_CAL_ERR    (1 << 3) // Calibration Error (Read Only)
#define NAU7802_CTRL2_CALS       (1 << 2) // Start Calibration

// Calibration Modes (CALMOD bits 1:0)
#define NAU7802_CALMOD_OFFSET_INT (0b00) // Internal Offset Calibration (Default)
#define NAU7802_CALMOD_OFFSET_SYS (0b10) // System Offset Calibration
#define NAU7802_CALMOD_GAIN_SYS   (0b11) // System Gain Calibration

// Conversion Rate Select Values (bits 4-6 of CTRL2)
#define NAU7802_RATE_10SPS       0b000    // 10 samples per second (DEFAULT)
#define NAU7802_RATE_20SPS       0b001    // 20 samples per second
#define NAU7802_RATE_40SPS       0b010    // 40 samples per second
#define NAU7802_RATE_80SPS       0b011    // 80 samples per second
#define NAU7802_RATE_320SPS      0b100    // 320 samples per second

// LDO Voltage Select (bits 3-5 of CTRL1)
#define NAU7802_LDO_VOLTAGE_MASK (0b111 << 3)
#define NAU7802_LDO_4V5          (0b000 << 3) // 4.5V
#define NAU7802_LDO_4V2          (0b001 << 3) // 4.2V
#define NAU7802_LDO_3V9          (0b010 << 3) // 3.9V
#define NAU7802_LDO_3V6          (0b011 << 3) // 3.6V
#define NAU7802_LDO_3V3          (0b100 << 3) // 3.3V
#define NAU7802_LDO_3V0          (0b101 << 3) // 3.0V (ADAFRUIT DEFAULT)
#define NAU7802_LDO_2V7          (0b110 << 3) // 2.7V
#define NAU7802_LDO_2V4          (0b111 << 3) // 2.4V

// Calibration Modes (CALMOD bits 1:0)
#define NAU7802_CALMOD_OFFSET_INT (0b00) // Internal Offset Calibration (Default)
#define NAU7802_CALMOD_OFFSET_SYS (0b10) // System Offset Calibration
#define NAU7802_CALMOD_GAIN_SYS   (0b11) // System Gain Calibration

#endif // NAU7802_REGS_HPP
