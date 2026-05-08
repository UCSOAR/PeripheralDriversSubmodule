/**
 ********************************************************************************
 * @file    nau7802.cpp
 * @author  Javier
 * @date    2025-11-25
 * @brief   Implementation of the NAU7802 driver.
 ********************************************************************************
 */

#include "NAU7802.hpp"
#include "main.h"
#include <cstdint>

#include "Task.hpp"
#include "CubeDefines.hpp"

NAU7802::NAU7802(I2C_Wrapper* i2c_pointer, std::uint8_t address)
  : _i2c(i2c_pointer), _deviceAddress(address) {}

bool NAU7802::isReady() {
        uint8_t pu_ctrl = 0;

        if (readRegister(NAU7802_REG_PU_CTRL, &pu_ctrl) != NauStatus::OK) {
                return false;
        }

        return (pu_ctrl & NAU7802_PU_CTRL_CR) != 0;
}

// Begin NAU7802 Startup Sequence
NauStatus NAU7802::begin(uint8_t initialGain) {

    // 1. Send a reset command
    if (reset() != NauStatus::OK) {
        return NauStatus::ERR_I2C; // Failed to reset
    }
    HAL_Delay(10);

    // 2. Power up the analog and digital sections
    uint8_t pu_ctrl = NAU7802_PU_CTRL_PUA | NAU7802_PU_CTRL_PUD;
    if (writeRegister(NAU7802_REG_PU_CTRL, pu_ctrl) != NauStatus::OK) {
        return NauStatus::ERR_I2C;
    }

    // 3. Wait for the Power Up Ready bit
    uint8_t attempts = 100;
    bool powerReady = false;
    while (attempts > 0) {
        if (readRegister(NAU7802_REG_PU_CTRL, &pu_ctrl) == NauStatus::OK) {
            if (pu_ctrl & NAU7802_PU_CTRL_PUR) {
                powerReady = true;
                break;
            }
        }
        HAL_Delay(1);
        attempts--;
    }

    if (!powerReady) {
        return NauStatus::ERR_NOT_READY;
    }

    pu_ctrl |= NAU7802_PU_CTRL_AVDDS; // Turn on internal LDO to power the load cell
    pu_ctrl |= NAU7802_PU_CTRL_CS;    // Set Cycle Start bit

    if (writeRegister(NAU7802_REG_PU_CTRL, pu_ctrl) != NauStatus::OK) {
        return NauStatus::ERR_I2C;
    }

    // 4. Set LDO voltage (3.0V is Adafruit's choice)
    uint8_t ctrl1;
    if (readRegister(NAU7802_REG_CTRL1, &ctrl1) != NauStatus::OK) {
        return NauStatus::ERR_I2C;
    }
    ctrl1 &= ~NAU7802_LDO_VOLTAGE_MASK;  // Clear LDO bits
    ctrl1 |= NAU7802_LDO_3V0;             // Set to 3.0V
    if (writeRegister(NAU7802_REG_CTRL1, ctrl1) != NauStatus::OK) {
        return NauStatus::ERR_I2C;
    }

    // 5. Set Gain to 128x (Adafruit default, better sensitivity)
    NauStatus status = setGain(NAU7802_GAIN_128X);
    if (status != NauStatus::OK) {
        return status;
    }

    // 6. Set conversion rate to 10 SPS (default Adafruit rate)
    uint8_t ctrl2;
    if (readRegister(NAU7802_REG_CTRL2, &ctrl2) != NauStatus::OK) {
        return NauStatus::ERR_I2C;
    }
    ctrl2 &= ~NAU7802_CTRL2_CRS_MASK;    // Clear conversion rate bits
    ctrl2 |= (NAU7802_RATE_10SPS << 4);  // Set to 10 SPS
    if (writeRegister(NAU7802_REG_CTRL2, ctrl2) != NauStatus::OK) {
        return NauStatus::ERR_I2C;
    }

    // Datasheet guidance: allow six conversion cycles after startup before trusting data.
    // At default 10SPS this corresponds to about 600ms.
    HAL_Delay(600);

    return NauStatus::OK;
}

NauStatus NAU7802::readSensor(NAU7802_OUT *dest) {
    if (dest == nullptr) {
        return NauStatus::ERR_INVALID_ARG;
    }

    uint8_t buffer[3];

    if (readRegisters(NAU7802_REG_ADC_B2, buffer, 3 ) != NauStatus::OK) {
        dest -> raw_reading = 0;
        return NauStatus::ERR_I2C; // Read failed
    }

    // --- DEBUG PRINTS START ---
    // SOAR_PRINT("NAU Driver - Bytes Read: B2: 0x%02X, B1: 0x%02X, B0: 0x%02X\n", buffer[0], buffer[1], buffer[2]);

    // Combine the three bytes
    int32_t value = ((int32_t)buffer[0] << 16) | \
                    ((int32_t)buffer[1] << 8)  | \
                    (buffer[2]);

    // SOAR_PRINT("NAU Driver - Combined (Pre-Sign Extend): 0x%08lX (%ld)\n", (unsigned long)value, (long)value);

    // Sign-extend the 24-bit value to a 32-bit integer
    if (value & 0x00800000) {
        value |= 0xFF000000;
    }

    // SOAR_PRINT("NAU Driver - Final Sign-Extended: 0x%08lX (%ld)\n", (unsigned long)value, (long)value);
    // --- DEBUG PRINTS END ---

    dest->raw_reading = value;
    return NauStatus::OK;
}

NauStatus NAU7802::reset() {
    // RR bit
    if (modifyRegisterBit(NAU7802_REG_PU_CTRL, NAU7802_PU_CTRL_RR, true) != NauStatus::OK) {
        return NauStatus::ERR_I2C;
    }

    HAL_Delay(1); // Small delay to ensure reset is processed

    // Clear RR bit
    return modifyRegisterBit(NAU7802_REG_PU_CTRL, NAU7802_PU_CTRL_RR, false);
}

NauStatus NAU7802::setGain(uint8_t gain) {
    if (gain > NAU7802_GAIN_128X) {
        return NauStatus::ERR_INVALID_ARG; // Invalid gain setting
    }

    // Read current CTRL1 register state
    uint8_t ctrl1;
    if (readRegister(NAU7802_REG_CTRL1, &ctrl1) != NauStatus::OK) {
        return NauStatus::ERR_I2C; // Failed to read CTRL1 register
    }

    // Modify gain bits
    ctrl1 &= 0b11111000; // Clear existing gain
    ctrl1 |= gain; // Set new gain

    // Write back modified CTRL1 register
    if (writeRegister(NAU7802_REG_CTRL1, ctrl1) != NauStatus::OK) {
        return NauStatus::ERR_I2C; // Failed to write CTRL1 register
    }

    // Verify the gain setting
    uint8_t verifyCtrl1;
    if (readRegister(NAU7802_REG_CTRL1, &verifyCtrl1) != NauStatus::OK) {
        return NauStatus::ERR_I2C; // Failed to read back CTRL1 register
    }

    // Check if gain bits match
    if ((verifyCtrl1 & 0b00000111) == gain) {
        return NauStatus::OK; // Success
    }
    else {
        return NauStatus::ERR_I2C; // Verification failed
    }
}

/* -- Private Helpers -- */
NauStatus NAU7802::writeRegister(std::uint8_t reg, std::uint8_t value) {
    if (_i2c->writeByte(_deviceAddress, reg, value)) {
        return NauStatus::OK;
    }
    return NauStatus::ERR_I2C;
}

NauStatus NAU7802::readRegister(std::uint8_t reg, std::uint8_t* value) {
    if (_i2c->readByte(_deviceAddress, reg, value)) {
        return NauStatus::OK;
    }
    return NauStatus::ERR_I2C;
}

NauStatus NAU7802::readRegisters(std::uint8_t reg, std::uint8_t* buffer, std::uint8_t len) {
    if (_i2c->readBytes(_deviceAddress, reg, buffer, len)) {
        return NauStatus::OK;
    }
    return NauStatus::ERR_I2C;
}

NauStatus NAU7802::modifyRegisterBit(std::uint8_t reg, std::uint8_t bitMask, bool state) {
    uint8_t val;
    
    // 1. Check Read Status
    NauStatus status = readRegister(reg, &val);
    if (status != NauStatus::OK) {
        return status; // Propagate error
    }

    // 2. Modify
    if (state) {
        val |= bitMask;
    } else {
        val &= ~bitMask;
    }

    // 3. Return Write Status
    return writeRegister(reg, val);
}

NauStatus NAU7802::calibrate() {
    if (modifyRegisterBit(NAU7802_REG_CTRL2, NAU7802_CTRL2_CALS, true) != NauStatus::OK) {
        return NauStatus::ERR_I2C;
    }

    constexpr unsigned int calibrationTimeoutMs = 1000;
    unsigned int elapsedMs = 0;

    while (elapsedMs < calibrationTimeoutMs) {
        uint8_t ctrl2 = 0;
        if (readRegister(NAU7802_REG_CTRL2, &ctrl2) != NauStatus::OK) {
            return NauStatus::ERR_I2C;
        }

        if ((ctrl2 & NAU7802_CTRL2_CALS) == 0) {
            if ((ctrl2 & NAU7802_CTRL2_CAL_ERR) != 0) {
                return NauStatus::ERR_NOT_READY;
            }
            return NauStatus::OK;
        }

        HAL_Delay(1);
        ++elapsedMs;
    }

    return NauStatus::ERR_TIMEOUT;
}
