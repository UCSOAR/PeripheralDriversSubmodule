/*
 * nau7802_regs.hpp
 *
 * Register definitions and constants for the MMC5983MA magnetometer.
 */
#ifndef mmc5983ma_regs_HPP
#define mmc5983ma_regs_HPP

//  Register Map
#define Xout0 0x00H             // Xout0 Register addresses
#define Xout1 0x01H             // Xout1 Register addresses
#define Yout0 0x02H             // Yout0 Register addresses
#define Yout1 0x03H             // Yout1 Register addresses
#define Zout0 0x04H             // Zout0 Register addresses
#define Zout1 0x05H             // Zout1 Register addresses
#define XYZout 0x06H            // XYZout Register addresses
#define Tout 0x07H              // Temperature Out Unsigned [ (-75~125C); LSB == -75 ]Register addresses
#define Status 0x08H            // Status Register addresses
#define It_Control0 0x09H       // Register addresses
#define It_Control1 0x0AH       // Register addresses
#define It_Control2 0x0BH       // Register addresses
#define It_Control3 0x0CH       // Register addresses
#define P_ID 0x2FH              // Register addresses

// Bit Masks for Status Register (0x08)
#define MEAS_M_DONE (1 << 0)   // Magnetic Field Data Ready Bit Mask
#define MEAS_T_DONE (1 << 1)   // Temperature Data Ready Bit Mask
#define OT_Rd_DONE  (1 << 4)   // Memory Read Successful Bit Mask

// Bit Masks for It_Control0 Register (0x09)
#define 


#endif // mmc5983ma_regs_HPP