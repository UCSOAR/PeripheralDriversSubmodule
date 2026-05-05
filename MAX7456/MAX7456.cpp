/*
 * wet_oil.c
 *
 *  Created on: Apr 1, 2026
 *      Author: Local user
 */




#include "MAX7456.hpp"


void MAX7456::_OSD_WriteReg(uint8_t addr, uint8_t val) {
	if(csport == NULL || cspin == 0) {
		return;
	}

	if(addr == OSD_REG_STAT) { //read only
		return;
	}

	if (OSD_REG_WAITS_FOR_CMSTAT(addr)) { //wait for character memory to be ready
		uint8_t i = 0;
		while (_OSD_ReadReg(OSD_REG_STAT) & OSD_REG_STAT_CMSTAT) {
			i++;
			if(i > 100) {
				return;
			}
			HAL_Delay(1);
		}
	}

	if (OSD_REG_WAITS_FOR_DMM(addr)) { //wait for dmm to be ready
		uint8_t i = 0;
		while (_OSD_ReadReg(OSD_REG_DMM) & OSD_REG_DMM_CLEAR) {
			i++;
			if(i > 100) {
				return;
			}
			HAL_Delay(1);
		}
	}

	if (OSD_REG_WAITS_FOR_OSD_OFF(addr)) { //check for osd to be off (won't happen on its own, user needs to do this)
		if(_OSD_ReadReg(OSD_REG_VM0) & OSD_REG_VM0_OSDDISP) {
			return;
		}
	}

	HAL_GPIO_WritePin(csport, cspin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(csport, cspin, GPIO_PIN_RESET);
	const uint8_t data[2] = {(uint8_t)(addr & 0b01111111),val};
	HAL_SPI_Transmit(&hspi2, data, 2, 1000);
	HAL_GPIO_WritePin(csport, cspin, GPIO_PIN_SET);
}

uint8_t MAX7456::_OSD_ReadReg(uint8_t addr) {
	if(csport == NULL || cspin == 0) {
		return 0x00;
	}
	HAL_GPIO_WritePin(csport, cspin, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(csport, cspin, GPIO_PIN_RESET);
	HAL_Delay(1);
	const uint8_t data[2] = {(uint8_t)(addr | 0b10000000),0x00};
	uint8_t rxdata[2] = {0x00,0x00};
	if(HAL_SPI_TransmitReceive(&hspi2, data, rxdata, 2, 1000) != HAL_OK) {
		HAL_GPIO_WritePin(csport, cspin, GPIO_PIN_SET);
		return 0x00;
	}

	HAL_GPIO_WritePin(csport, cspin, GPIO_PIN_SET);

	return rxdata[1];
}

uint8_t GetBitIndexOfLowest1(uint8_t val) {
	uint8_t i = 0;
	while(!(val&1) && i < 8) {
		i++;
		val>>=1;
	}
	return i;
}

void MAX7456::_OSD_WriteField(uint8_t addr, uint8_t mask, uint8_t val) {
	uint8_t currval = _OSD_ReadReg(addr);

	currval = (currval & ~mask) | ((val << GetBitIndexOfLowest1(mask)) & mask);

	_OSD_WriteReg(addr, currval);
}

uint8_t MAX7456::_OSD_ReadField(uint8_t addr, uint8_t mask) {
	uint8_t currval = _OSD_ReadReg(addr);
	return (currval & mask) >> GetBitIndexOfLowest1(mask);
}


void MAX7456::OSD_SetStandard(enum OSD_STANDARD standard) {
	_OSD_WriteField(OSD_REG_VM0, OSD_REG_VM0_STANDARDSELECT, standard);
}

void MAX7456::OSD_SetOSDEnabled(uint8_t enabled) {
	_OSD_WriteField(OSD_REG_VM0, OSD_REG_VM0_OSDDISP, enabled > 0);
}

void MAX7456::OSD_Reset() {
	_OSD_WriteField(OSD_REG_VM0, OSD_REG_VM0_SOFTRESET, 1);

	uint16_t i = 0;
	while(_OSD_ReadField(OSD_REG_VM0, OSD_REG_VM0_SOFTRESET) && i < 250) {
		HAL_Delay(1);
		i++;
	}
}

void MAX7456::OSD_SetVideoEnabled(uint8_t enabled) {
	_OSD_WriteField(OSD_REG_VM0, OSD_REG_VM0_VIDEO_ENABLE,enabled==0);
}

void MAX7456::OSD_SetBackgroundMode(enum OSD_BACKGROUND_MODE mode) {
	_OSD_WriteField(OSD_REG_VM1, OSD_REG_VM1_BACKMODE,mode);
}

void MAX7456::OSD_SetBackgroundBrightness(uint8_t level) {
	_OSD_WriteField(OSD_REG_VM1, OSD_REG_VM1_BACKBRIGHTNESS,level & 0b00000111);
}

void MAX7456::OSD_SetHorizontalOffset(uint8_t offset) {
	_OSD_WriteField(OSD_REG_HOS, OSD_REG_HOS_HPOS, offset&0b00111111);
}

void MAX7456::OSD_SetVerticalOffset(uint8_t offset) {
	_OSD_WriteField(OSD_REG_VOS, OSD_REG_VOS_VPOS, offset&0b00011111);
}

void MAX7456::OSD_SetBitMode(enum OSD_BITMODE bitmode) {
	_OSD_WriteField(OSD_REG_DMM, OSD_REG_DMM_OPMODE,bitmode);
}

void MAX7456::OSD_SetLocalBackgroundMode(enum OSD_LBC_MODE mode) {
	_OSD_WriteField(OSD_REG_DMM, OSD_REG_DMM_LBC,mode);
}

void MAX7456::OSD_SetBlinking(uint8_t blinking) {
	_OSD_WriteField(OSD_REG_DMM, OSD_REG_DMM_BLK,blinking>0);
}

void MAX7456::OSD_SetInverted(uint8_t inverted) {
	_OSD_WriteField(OSD_REG_DMM, OSD_REG_DMM_INV,inverted > 0);
}

void MAX7456::OSD_ClearDispMem() {
	_OSD_WriteField(OSD_REG_DMM, OSD_REG_DMM_CLEAR,1);
}

void MAX7456::OSD_SetAutoIncrementMode(uint8_t autoinc) {
	_OSD_WriteField(OSD_REG_DMM, OSD_REG_DMM_AUTOINC,autoinc>0);
}

void MAX7456::OSD_SetDMByteMode(enum OSD_DM_BYTE_SEL mode) {
	_OSD_WriteField(OSD_REG_DMAH, OSD_REG_DMAH_BYTESEL,mode);
}

void MAX7456::OSD_SetDMAddress(uint16_t addr) {
	_OSD_WriteField(OSD_REG_DMAH, OSD_REG_DMAH_BIT8,(addr>>8)&1);
	_OSD_WriteField(OSD_REG_DMAL, OSD_REG_DMAL_DMADD,addr&0xff);
}

void MAX7456::OSD_LoadDMData(uint8_t data) {
	_OSD_WriteField(OSD_REG_DMDI, OSD_REG_DMDI_DATA,data);
}

void MAX7456::OSD_SelectCharacter(uint8_t character) {
	_OSD_WriteField(OSD_REG_CMAH, OSD_REG_CMAH_DATA,character);
}

void MAX7456::OSD_SelectCharacterAddress(uint8_t charaddr) {
	_OSD_WriteField(OSD_REG_CMAL, OSD_REG_CMAL_DATA,charaddr & 0b00111111);
}

uint8_t MAX7456::OSD_Status() {
	return _OSD_ReadReg(OSD_REG_STAT);
}

void MAX7456::OSD_SetOSDBL(uint8_t enabled) {
	_OSD_WriteField(OSD_REG_OSDBL, OSD_REG_OSDBL_EN, enabled == 0);
}

void MAX7456::OSD_WriteString(const char* str, uint8_t x, uint8_t y) {
	if(y > 15 || x > 29) {
		return;
	}
	OSD_SetAutoIncrementMode(0);
	OSD_SetBitMode(OSD_BITMODE_8);
	OSD_SetDMByteMode(OSD_DM_BYTE_ADDRESS);


	for (const char* i = str; *i != 0x00; i++) {
		uint16_t addr = y*30+x+(i-str);
		if(addr > 479) {
			return;
		}
		OSD_SetDMAddress(y*30+x+(i-str));
		OSD_LoadDMData(OSD_ConvertASCIIToCharacterAddress(*i));
	}

}


void MAX7456::OSD_WriteCustomCharacter(uint8_t char_index, const uint8_t* pixel_data) {

	// stays disabled if started disabled
	uint8_t wasEnabled = _OSD_ReadField(OSD_REG_VM0, OSD_REG_VM0_OSDDISP);
	if (wasEnabled) {
		OSD_SetOSDEnabled(0);
	}

	_OSD_WriteReg(OSD_REG_CMAH, char_index);


	for (uint8_t i = 0; i < 54; i++) {
		_OSD_WriteReg(OSD_REG_CMAL, i);
		_OSD_WriteReg(OSD_REG_CMDI, pixel_data[i]);
	}


	_OSD_WriteReg(OSD_REG_CMM, OSD_REG_CMM_WRITE_NVM);

	uint16_t timeout = 0;
	while (_OSD_ReadReg(OSD_REG_STAT) & OSD_REG_STAT_CMSTAT) {
		HAL_Delay(1);
		timeout++;
		if(timeout > 200) {
			break; // usually takes ~12ms
		}
	}

	if (wasEnabled) {
		OSD_SetOSDEnabled(1);
	}
}

void MAX7456::OSD_DrawLogo(uint8_t startCharIndex, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
	if(y > (16-h) || x > (30-w) || w > 30 || h > 16) {
		return;
	}

	OSD_SetAutoIncrementMode(0);
	OSD_SetBitMode(OSD_BITMODE_8);
	OSD_SetDMByteMode(OSD_DM_BYTE_ADDRESS);

	uint8_t current_char = startCharIndex;

	for (uint8_t row = 0; row < h; row++) {
		for (uint8_t col = 0; col < w; col++) {
			uint16_t addr = (y+row) * 30 + (x + col);

			OSD_SetDMAddress(addr);
			OSD_LoadDMData(current_char);

			current_char++;
		}
	}
}


MAX7456::MAX7456(GPIO_TypeDef* csgpio, uint16_t cspin) : csport(csgpio),cspin(cspin) {

}
