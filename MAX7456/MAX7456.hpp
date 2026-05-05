/*
 * wet_oil.h
 *
 *  Created on: Apr 1, 2026
 *      Author: Local user
 */

#ifndef INC_WET_OIL_H_
#define INC_WET_OIL_H_

#include "gpio.h"
#include "spi.h"



//static struct _OSDInfo osdInfo;

#define OSD_REG_VM0 0x00

#define OSD_REG_VM0_STANDARDSELECT (1<<6)
#define OSD_REG_VM0_OSDDISP (1<<3)
#define OSD_REG_VM0_SOFTRESET (1<<1)
#define OSD_REG_VM0_VIDEO_ENABLE (1<<0)

#define OSD_REG_VM1 0x01
#define OSD_REG_VM1_BACKMODE (1<<7)
#define OSD_REG_VM1_BACKBRIGHTNESS (0b01110000)

#define OSD_REG_HOS 0x02
#define OSD_REG_HOS_HPOS (0b00111111)

#define OSD_REG_VOS 0x03
#define OSD_REG_VOS_VPOS (0b00011111)

#define OSD_REG_DMM 0x04
#define OSD_REG_DMM_OPMODE (1<<6)
#define OSD_REG_DMM_LBC (1<<5)
#define OSD_REG_DMM_BLK (1<<4)
#define OSD_REG_DMM_INV (1<<3)
#define OSD_REG_DMM_CLEAR (1<<2)
#define OSD_REG_DMM_AUTOINC (1<<0)

#define OSD_REG_DMAH 0x05
#define OSD_REG_DMAH_BYTESEL (1<<1)
#define OSD_REG_DMAH_BIT8 (1<<0)

#define OSD_REG_DMAL 0x06
#define OSD_REG_DMAL_DMADD (0b11111111)

#define OSD_REG_DMDI 0x07
#define OSD_REG_DMDI_DATA (0b11111111)

#define OSD_REG_CMM  0x08
#define OSD_REG_CMM_WRITE_NVM 0xA0 // Command to execute NVM write

#define OSD_REG_CMDI 0x0B

#define OSD_REG_CMAH 0x09
#define OSD_REG_CMAH_DATA (0b11111111)

#define OSD_REG_CMAL 0x0A
#define OSD_REG_CMAL_DATA (0b00111111)

#define OSD_REG_STAT 0xA0
#define OSD_REG_STAT_RESETMODE (1<<6)
#define OSD_REG_STAT_CMSTAT (1<<5)
#define OSD_REG_STAT_NTSC_DETECTED (1<<1)
#define OSD_REG_STAT_PAL_DETECTED (1<<0)

#define OSD_REG_OSDBL 0x6C
#define OSD_REG_OSDBL_EN (1<<4)

class MAX7456 {

public:

	MAX7456(GPIO_TypeDef* csgpio, uint16_t cspin);
	enum OSD_STANDARD {
		OSD_STANDARD_NTSC,
		OSD_STANDARD_PAL
	};

	void OSD_SetStandard(enum OSD_STANDARD standard);
	void OSD_SetOSDEnabled(uint8_t enabled);
	void OSD_Reset();
	void OSD_SetVideoEnabled(uint8_t enabled);

	enum OSD_BACKGROUND_MODE {
		OSD_BACKGROUND_LBC_CONTROLLED,
		OSD_BACKGROUND_GLOBAL
	};
	void OSD_SetBackgroundMode(enum OSD_BACKGROUND_MODE mode);

	void OSD_SetBackgroundBrightness(uint8_t level);

	void OSD_SetHorizontalOffset(uint8_t offset);

	void OSD_SetVerticalOffset(uint8_t offset);

	enum OSD_BITMODE {
		OSD_BITMODE_8,
		OSD_BITMODE_16
	};
	void OSD_SetBitMode(enum OSD_BITMODE bitmode);

	enum OSD_LBC_MODE {
		OSD_LBC_CLEAR,
		OSD_LBC_GRAY
	};
	void OSD_SetLocalBackgroundMode(enum OSD_LBC_MODE mode);

	void OSD_SetBlinking(uint8_t blinking);

	void OSD_SetInverted(uint8_t inverted);

	void OSD_ClearDispMem();

	void OSD_SetAutoIncrementMode(uint8_t autoinc);

	enum OSD_DM_BYTE_SEL {
		OSD_DM_BYTE_ADDRESS,
		OSD_DM_BYTE_ATTRIBUTE
	};

	void OSD_SetDMByteMode(enum OSD_DM_BYTE_SEL mode);

	void OSD_SetDMAddress(uint16_t addr);

	void OSD_LoadDMData(uint8_t data);

	void OSD_SelectCharacter(uint8_t character);

	void OSD_SelectCharacterAddress(uint8_t charaddr);

	void OSD_SetOSDBL(uint8_t enabled);

	uint8_t OSD_Status();

	void OSD_WriteString(const char* str, uint8_t x, uint8_t y);


	void OSD_WriteCustomCharacter(uint8_t char_index, const uint8_t* pixel_data);
	void OSD_DrawLogo(uint8_t startCharIndex, uint8_t x, uint8_t y, uint8_t w, uint8_t h);


private:
	void _OSD_WriteReg(uint8_t addr, uint8_t val);
	uint8_t _OSD_ReadReg(uint8_t addr);

	void _OSD_WriteField(uint8_t addr, uint8_t mask, uint8_t val);
	uint8_t _OSD_ReadField(uint8_t addr, uint8_t mask);
	static inline int OSD_REG_WAITS_FOR_CMSTAT(uint8_t addr) {
		return addr == OSD_REG_VM0 || addr == OSD_REG_CMAH || addr == OSD_REG_CMAL;
	}

	static inline int OSD_REG_WAITS_FOR_DMM(uint8_t addr) {
		return addr == OSD_REG_VM0 || addr == OSD_REG_CMAH || addr == OSD_REG_CMAL || addr == OSD_REG_DMM || addr == OSD_REG_DMAH || addr == OSD_REG_DMAL || addr == OSD_REG_DMDI;
	}

	static inline int OSD_REG_WAITS_FOR_OSD_OFF(uint8_t addr) {
		return addr == OSD_REG_CMAH || addr == OSD_REG_CMAL;
	}


	static inline uint8_t OSD_ConvertASCIIToCharacterAddress(char c) {
		static const char OSD_ASCIITABLE[] = {'\x00','1','2','3','4','5','6','7','8','9','0','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','(',')','.','?',';',':',',','\'','/','"','-','<','>','@'};

		uint8_t i = 0;
		while (i < sizeof(OSD_ASCIITABLE) / sizeof(char)){
			if(OSD_ASCIITABLE[i] == c) {
				return i;
			}
			i++;
		}
		return 0;

	}

	GPIO_TypeDef* csport;
	const uint16_t cspin;

};








#endif /* INC_WET_OIL_H_ */
