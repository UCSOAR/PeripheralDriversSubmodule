/**
 ********************************************************************************
 * @file    MX66L1G45GMI.hpp
 * @author  shiva
 * @date    Mar 26, 2025
 * @brief
 ********************************************************************************
 */

#ifndef MX66L1G45GMI_HPP_
#define MX66L1G45GMI_HPP_

/************************************
 * INCLUDES
 ************************************/
#include <stdbool.h>
#include <stdint.h>
/************************************
 * MACROS AND DEFINES
 ************************************/
#ifdef __cplusplus
extern "C"
{
#endif

// MX66L1G45GMI Flash Memory Specifications
#define FS_PAGE_SIZE 256         // Page size for programming (256 bytes)
#define FS_SECTOR_SIZE 4096      // Sector size for erase (4KB)
#define FS_BLOCK_SIZE 32768      // Block size for erase (32KB)
#define FS_TOTAL_SIZE 0x08000000 // Total: 128MB (1Gb / 8 bits per byte)

    /************************************
     * TYPEDEFS
     ************************************/

    /************************************
     * CLASS DEFINITIONS
     ************************************/

    /************************************
     * FUNCTION DECLARATIONS
     ************************************/

    uint32_t MX66_ReadID(void);
    uint8_t MX66_ReadStatus(int reg); // Read status reg1,2,3
    void MX66_WriteStatus(int reg, uint8_t newstatus);

    void MX66_ReadSFDP(uint8_t *rData);
    void MX66_ReleaseFromDeepPowerDown(void);
    void MX66_Read(uint32_t block, uint16_t offset, uint32_t size, uint8_t *rData);
    void MX66_FastRead(uint32_t block, uint16_t offset, uint32_t size, uint8_t *rData);

    void write_enable(void);
    void write_disable(void);

    void MX66_Erase_Chip(void);
    void MX66_Erase_Sector(uint16_t numsector);
    void MX66_Erase_Block(uint32_t numblock);

    void MX66_Write_Page(uint32_t page, uint16_t offset, uint32_t size, const uint8_t *data);
    void MX66_Write_Block(uint32_t block, uint16_t offset, uint32_t size, const uint8_t *data);

    bool isWriteProtected(void);

    // Low-level helper APIs used by the driver implementation
    void MX66_Delay(uint32_t time);
    void csLOW(void);
    void csHIGH(void);
    void SPI_Write(uint8_t *data, uint16_t len);
    void SPI_Read(uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* MX66L1G45GMI_HPP_ */
