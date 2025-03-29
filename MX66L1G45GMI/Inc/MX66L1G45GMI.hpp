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

/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/
uint32_t MX66_ReadID (void);

uint8_t MX66_ReadStatus(int reg);								// Read status reg1,2,3

void MX66_WriteStatus(int reg, uint8_t newstatus);

void MX66_ReadSFDP(uint8_t *rData);

void MX66_Read (uint32_t block, uint16_t offset, uint32_t size, uint8_t *rData);

void MX66_FastRead (uint32_t block, uint16_t offset, uint32_t size, uint8_t *rData);

void write_enable(void);

void write_disable(void);

void MX66_Erase_Chip(void);

void MX66_Erase_Sector(uint16_t numsector);

void MX66_Erase_Block(uint32_t numblock);













#endif /* MX66L1G45GMI_HPP_ */
