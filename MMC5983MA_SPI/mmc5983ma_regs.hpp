/*
 * mmc5983ma_regs.hpp
 * WIP | Feat mmc5983ma_regs - Part 1
 * Register definitions and constants for the MMC5983MA magnetometer.
 */
#ifndef MMC5983MA_REGS_HPP
#define MMC5983MA_REGS_HPP

//  Register Map
#define MMC5983MA_XOUT0 0x00             // Xout0 Register addresses
#define MMC5983MA_XOUT1 0x01             // Xout1 Register addresses
#define MMC5983MA_YOUT0 0x02             // Yout0 Register addresses
#define MMC5983MA_YOUT1 0x03             // Yout1 Register addresses
#define MMC5983MA_ZOUT0 0x04             // Zout0 Register addresses
#define MMC5983MA_ZOUT1 0x05             // Zout1 Register addresses
#define MMC5983MA_XYZOUT 0x06            // XYZout Register addresses
#define MMC5983MA_TOUT 0x07              // Temperature Out Unsigned [ (-75~125C); LSB == -75 ]Register addresses
#define MMC5983MA_STATUS 0x08            // Status Register addresses
#define MMC5983MA_IT_CONTROL0 0x09       // Register addresses
#define MMC5983MA_IT_CONTROL1 0x0A       // Register addresses
#define MMC5983MA_IT_CONTROL2 0x0B       // Register addresses
#define MMC5983MA_IT_CONTROL3 0x0C       // Register addresses
#define MMC5983MA_P_ID 0x2F              // Register addresses

// Bit Masks for Status Register (0x08)
#define MMC5983MA_MEAS_M_DONE (1 << 0)   // Magnetic Field Data Ready Bit Mask
#define MMC5983MA_MEAS_T_DONE (1 << 1)   // Temperature Data Ready Bit Mask
#define MMC5983MA_OTP_RD_DONE  (1 << 4)   // Memory Read Successful Bit Mask

// Bit Masks for IT_Control0 Register (0x09)
#define MMC5983MA_TM_M                     (1 << 0)   // MF Measurement Bit Mask
#define MMC5983MA_TM_T                     (1 << 1)   // T Measurement Bit Mask
#define MMC5983MA_INT_MEAS_DONE_EN         (1 << 2)   // Interrupt Measurement Done Enable Bit Mask
#define MMC5983MA_SET                      (1 << 3)   // Setting Operation Bit Mask
#define MMC5983MA_RESET                    (1 << 4)   // Soft Reset Bit Mask
#define MMC5983MA_AUTO_SR_EN               (1 << 5)   // Auto Self-Reset Enable Bit Mask
#define MMC5983MA_OTP_READ                 (1 << 6)   // One-Time Programmable Read Bit Mask

// Bit Masks for IT_Control1 Register (0x0A)
// Bandwidth (BW[1:0]) settings: measurement time and data output rate
#define MMC5983MA_BW_100HZ (0x00) // 00: 8ms, 100Hz
#define MMC5983MA_BW_200HZ (0x01) // 01: 4ms, 200Hz
#define MMC5983MA_BW_400HZ (0x02) // 10: 2ms, 400Hz
#define MMC5983MA_BW_800HZ (0x03) // 11: 0.5ms, 800Hz
// Channel inhibits
#define MMC5983MA_X_INHIBIT (1 << 1)  // Disable X channel
#define MMC5983MA_Y_INHIBIT ((1 << 3) | (1 << 4))   // Disable Y and Z channels
// Software Reset
#define MMC5983MA_SW_RST (1 << 7) // Software Reset


// Bit Masks for IT_Control2 Register (0x0B)
// Continuous Measurement Frequency (CM_Freq[2:0])
#define MMC5983MA_CM_FREQ_OFF (0x00)    // 000: Continuous Mode Off
#define MMC5983MA_CM_FREQ_1Z (0x01)    // 001: 1Hz
#define MMC5983MA_CM_FREQ_10Z (0x02)   // 010: 10Hz
#define MMC5983MA_CM_FREQ_20Z (0x03)   // 011: 20Hz
#define MMC5983MA_CM_FREQ_50Z (0x04)   // 100: 50Hz
#define MMC5983MA_CM_FREQ_100Z (0x05)  // 101: 100Hz
#define MMC5983MA_CM_FREQ_200Z (0x06)  // 110: 200Hz (BW=01)
#define MMC5983MA_CM_FREQ_1000Z (0x07) // 111: 1000Hz (BW=11)
// Continuous Measurement Enable
#define MMC5983MA_CMM_EN (1 << 3) // Enable Continuous Mode
// Periodic Set (Prd_set[2:0]) - shifted to bits [6:4]
#define MMC5983MA_PRD_SET_1 (0x00 << 4)    // 000: Every 1 measurement
#define MMC5983MA_PRD_SET_25 (0x01 << 4)   // 001: Every 25 measurements
#define MMC5983MA_PRD_SET_75 (0x02 << 4)   // 010: Every 75 measurements
#define MMC5983MA_PRD_SET_100 (0x03 << 4)  // 011: Every 100 measurements
#define MMC5983MA_PRD_SET_250 (0x04 << 4)  // 100: Every 250 measurements
#define MMC5983MA_PRD_SET_500 (0x05 << 4)  // 101: Every 500 measurements
#define MMC5983MA_PRD_SET_1000 (0x06 << 4) // 110: Every 1000 measurements
#define MMC5983MA_PRD_SET_2000 (0x07 << 4) // 111: Every 2000 measurements
// Periodic Set Enable
#define MMC5983MA_EN_PRD_SET (1 << 7) // Enable Periodic Set


// Bit Masks for IT_Control3 Register (0x0C)
#define MMC5983MA_ST_ENP (1 << 2) // Self-test positive current
#define MMC5983MA_ST_ENM (1 << 3) // Self-test negative current
#define MMC5983MA_SPI_3W (1 << 6) // Enable 3-wire SPI mode


// P_ID Register (0x2F)
#define MMC5983MA_PRODUCT_ID_VALUE 0x28 // Default value of Product ID reg

#endif // MMC5983MA_REGS_HPP