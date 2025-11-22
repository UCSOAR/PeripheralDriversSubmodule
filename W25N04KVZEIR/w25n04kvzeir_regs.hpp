/*
 * w25n04kvzeir_regs.hpp
 */
#ifndef x_REGS_HPP
#define x_REGS_HPP

/*
 *  ================================================
 *  Register Map Buffer Read, BUF = 1, default
 *  ================================================
 */

// Device Identification Registers
#define W25N04KVZEIR_DID 0xEF   // Device ID
#define W25N04KVZEIR_MID 0xAA23 // Manufacturer ID

// Reset & JEDEC ID
#define W25N04KVZEIR_RESET 0xFF    // Reset Command
#define W25N04KVZEIR_JEDEC_ID 0x9F // JEDEC ID

/*------------------------------------------------
 * Status Register-1 Bit Definitions
 * Address: 0F / __
 * Datasheet Section 7.2
 * ------------------------------------------------*/

#define W25N04KVZEIR_READ_STATUS_REG_1 0x0F05            // Combined Read Status Registers 1 and 2
#define W25N04KVZEIR_READ_STATUS_REG_2 0x05              // Read Status Register 2
#define W25N04KVZEIR_READ_STATUS_REG_3 0x05              // Read Status Register 3
#define W25N04KVZEIR_READ_EXTENDED_INTERNAL_ECC_REG 0x05 // Read Extended Internal ECC Register

/*------------------------------------------------
 * Status Register-2 Bit Definitions
 * Address: 1F / __
 * Datasheet Section 7.2
 * ------------------------------------------------*/

#define W25N04KVZEIR_WRITE_STATUS_REG_1 0x01              // Write Status Register 1
#define W25N04KVZEIR_WRITE_STATUS_REG_2 0x01              // Write Status Register 2
#define W25N04KVZEIR_WRITE_EXTENDED_INTERNAL_ECC_REG 0xC0 // Write Extended Internal ECC Register

// Write Enable/Disable
#define W25N04KVZEIR_WRITE_ENABLE 0x06  // Write Enable
#define W25N04KVZEIR_WRITE_DISABLE 0x04 // Write Disable

// Block/Chip Erase
#define W25N04KVZEIR_BLOCK_ERASE 0xD8 // Block Erase (128KB)

// Program
#define W25N04KVZEIR_RANDOM_PROGRAM_RESET_BUFFER 0x02   // Random Program Reset Buffer
#define W25N04KVZEIR_RANDOM_PROGRAM_DATA_LOAD 0x84      // Random Program Data Load
#define W25N04KVZEIR_QUAD_INPUT_PAGE_PROGRAM 0x32       // Quad Input Page Program
#define W25N04KVZEIR_RANDOM_QUAD_PROGRAM_DATA_LOAD 0x34 // Random Quad Program Data Load
#define W25N04KVZEIR_PROGRAM_EXECUTE 0x10               // Program Execute
// Read
#define W25N04KVZEIR_PAGE_DATA_READ 0x13  // Page Data Read
#define W25N04KVZEIR_READ 0x03            // Read
#define W25N04KVZEIR_FAST_READ 0x0B       // Fast Read
#define W25N04KVZEIR_FAST_READ_4BYTE 0x0C // Fast Read 4 Byte Address

// Dual Output
#define W25N04KVZEIR_DUAL_OUTPUT_FAST_READ 0x3B       // Dual Output Fast Read
#define W25N04KVZEIR_DUAL_OUTPUT_FAST_READ_4BYTE 0x3C // Dual Output Fast Read 4 Byte Address
#define W25N04KVZEIR_DUAL_IO_FAST_READ 0xBB           // Dual IO Fast Read
#define W25N04KVZEIR_DUAL_IO_FAST_READ_4BYTE 0xBC     // Dual IO Fast Read 4 Byte Address

// Quad Output
#define W25N04KVZEIR_QUAD_OUTPUT_FAST_READ 0x6B       // Quad Output Fast Read
#define W25N04KVZEIR_QUAD_OUTPUT_FAST_READ_4BYTE 0x6C // Quad Output Fast Read 4 Byte Address
#define W25N04KVZEIR_QUAD_IO_FAST_READ 0xEB           // Quad IO Fast Read
#define W25N04KVZEIR_QUAD_IO_FAST_READ_4BYTE 0xEC     // Quad IO Fast Read 4 Byte Address

// Power Down / Reset
#define W25N04KVZEIR_POWER_DOWN 0xB9              // Power Down
#define W25N04KVZEIR_RELEASE_FROM_POWER_DOWN 0xAB // Release from Power Down
#define W25N04KVZEIR_ENABLE_RESET 0x66            // Enable Reset
#define W25N04KVZEIR_RESET_DEVICE 0x99            // Reset Device



/*------------------------------------------------
 * Status Register-1 (Configuration) Bit Definitions
 * Address: Axh
 * Datasheet Section 7.1
 * ------------------------------------------------*/

#define W25N04KVZEIR_SRP0      (1 << 7) 
#define W25N04KVZEIR_BP3       (1 << 6)
#define W25N04KVZEIR_BP2       (1 << 5)
#define W25N04KVZEIR_BP1       (1 << 4)
#define W25N04KVZEIR_BP0       (1 << 3)
#define W25N04KVZEIR_TB        (1 << 2)
#define W25N04KVZEIR_WP_E      (1 << 1)
#define W25N04KVZEIR_SRP1      (1 << 0)


/*------------------------------------------------
 * Status Register-2 (Configuration) Bit Definitions
 * Address: Bxh
 * Datasheet Section 7.2
 * ------------------------------------------------*/

#define W25N04KVZEIR_SR2_OTP_L (1 << 7) // OTP Data Pages Lock
#define W25N04KVZEIR_SR2_OTP_E (1 << 6) // Enter OTP Mode
#define W25N04KVZEIR_SR2_SR1_L (1 << 5) // Status Register-1 Lock
#define W25N04KVZEIR_SR2_ECC_E (1 << 4) // Enable ECC (Should be 1)
#define W25N04KVZEIR_SR2_BUF   (1 << 3) // Buffer Mode (1=Buffer, 0=Sequential)
#define W25N04KVZEIR_SR2_ODS_1 (1 << 2) // Output Driver Strength Bit 1
#define W25N04KVZEIR_SR2_ODS_0 (1 << 1) // Output Driver Strength Bit 0
#define W25N04KVZEIR_SR2_H_DIS (1 << 0) // Hold Disable

/*------------------------------------------------
 * Status Register-3 Bit Definitions
 * Address: Cxh
 * Datasheet Section 7.3
 * ------------------------------------------------*/
#define W25N04KVZEIR_ECC_1_0    ((1 << 5) | (1 << 4)) // ECC Status
#define W25N04KVZEIR_P_FAIL     (1 << 3) // Program Fail
#define W25N04KVZEIR_E_FAIL     (1 << 2) // Erase Fail
#define W25N04KVZEIR_WEL        (1 << 2) // Write Enable Latch
#define W25N04KVZEIR_BUSY       (1 << 1) // Operation in Progress



#endif // W25N04KVZEIR_REGS_HPP