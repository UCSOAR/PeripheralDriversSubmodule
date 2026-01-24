
//-------------------------------------------------------------------------------------------------
// STM32 SPI Driver
//-------------------------------------------------------------------------------------------------

void W25Q_Delay(uint32_t time)
{
	HAL_Delay(time);
}

void csLOW (void)
{
	HAL_GPIO_WritePin(SPI_FLASH_CS_GPIO_Port, SPI_FLASH_CS_Pin, GPIO_PIN_RESET);
}

void csHIGH (void)
{
	HAL_GPIO_WritePin(SPI_FLASH_CS_GPIO_Port, SPI_FLASH_CS_Pin, GPIO_PIN_SET);
}

void SPI_Write (uint8_t *data, uint16_t len)
{
	HAL_SPI_Transmit(&W25Q_SPI, data, len, 2000);
}

void SPI_Read (uint8_t *data, uint16_t len)
{
	HAL_SPI_Receive(&W25Q_SPI, data, len, 5000);
}

/**************************************************************************************************/

void W25Q_Reset (void)
{
	uint8_t tData[2];
	tData[0] = 0x66;  												// enable Reset
	tData[1] = 0x99;  												// Reset
	csLOW();
	SPI_Write(tData, 2);
	csHIGH();
	W25Q_Delay(100);
}

uint32_t W25Q_ReadID (void)
{
	uint8_t tData = 0x9F;  // Read JEDEC ID
	uint8_t rData[3];
	csLOW();
	SPI_Write(&tData, 1);
	SPI_Read(rData, 3);
	csHIGH();
	return ((rData[0]<<16)|(rData[1]<<8)|rData[2]);
}

uint8_t W25Q_ReadStatus(int reg)									// Read status reg1,2,3
{
	uint8_t tData,rData;
	switch(reg){
		case 1: tData = 0x05; break;
		case 2: tData = 0x35; break;
		case 3: tData = 0x15; break;
		default:
			printf("Invalid status register 0\n");
			return 0;
	}
	csLOW();
	SPI_Write(&tData, 1);
	SPI_Read(&rData, 1);
	csHIGH();
	return (rData);
}

void W25Q_WriteStatus(int reg, uint8_t newstatus)
{
	uint8_t tData[2];
	switch(reg){
		case 1: tData[0] = 0x01; break;
		case 2: tData[0] = 0x31; break;
		case 3: tData[0] = 0x11; break;
		default: return;
	}

	tData[1]=newstatus;
	write_enable();
	csLOW();
	SPI_Write(tData, 2);
	csHIGH();
	write_disable();
}

uint64_t W25Q_ReadUniqueID(void)
{
	uint8_t tData[5];  												// Read Unique 64bits  ID
	uint8_t rData[8];

	tData[0] = 0x4B;
	csLOW();
	SPI_Write(tData, 5);
	SPI_Read(rData, 8);
	csHIGH();
	printf("64bits Identifier = 0x");
	for (int i=0;i<8;i++) printf("%02x",rData[i]);
	printf("\n");

	return (((uint64_t)rData[0]<<56)|((uint64_t)rData[1]<<48)|((uint64_t)rData[2]<<40)|((uint64_t)rData[3]<<32)|
			((uint64_t)rData[4]<<24)|((uint64_t)rData[5]<<16)|((uint64_t)rData[6]<<8)|(uint64_t)rData[7]);
}

void W25Q_ReadSFDP(uint8_t *rData)
{
	uint8_t tData[5]={0x5A,0,0,0,0};
	csLOW();  														// pull the CS Low
	SPI_Write(tData, 5);
	SPI_Read(rData, 256);  											// Read the data
	csHIGH();  														// pull the CS High
}

void W25Q_Read (uint32_t block, uint16_t offset, uint32_t size, uint8_t *rData)
{
	uint8_t tData[6];
	uint32_t memAddr = (block*FS_SECTOR_SIZE) + offset;

	tData[0] = 0x03;  												// enable Read
	tData[1] = (memAddr>>16)&0xFF;  								// MSB of the memory Address
	tData[2] = (memAddr>>8)&0xFF;
	tData[3] = (memAddr)&0xFF; 										// LSB of the memory Address

	csLOW();  														// pull the CS Low
	SPI_Write(tData, 4);  											// 24 bit memory address
	SPI_Read(rData, size);  										// Read the data
	csHIGH();  														// pull the CS High
}

void W25Q_FastRead (uint32_t block, uint16_t offset, uint32_t size, uint8_t *rData)
{
	uint8_t tData[6];
	uint32_t memAddr = (block*FS_SECTOR_SIZE) + offset;

	tData[0] = 0x0B;  												// enable Fast Read
	tData[1] = (memAddr>>16)&0xFF;  								// MSB of the memory Address
	tData[2] = (memAddr>>8)&0xFF;
	tData[3] = (memAddr)&0xFF; 										// LSB of the memory Address
	tData[4] = 0;  													// Dummy clock

	csLOW();  														// pull the CS Low
	SPI_Write(tData, 5);  											// 24 bit memory address
	SPI_Read(rData, size);  										// Read the data
	csHIGH();  														// pull the CS High
}

void write_enable(void)
{
	uint8_t tData = 0x06;  											// enable write
	csLOW();
	SPI_Write(&tData, 1);
	csHIGH();
	W25Q_Delay(5);  												// 5ms delay
}

void write_disable(void)
{
	uint8_t tData = 0x04;  											// disable write
	csLOW();
	SPI_Write(&tData, 1);
	csHIGH();
	W25Q_Delay(5);  												// 5ms delay
}

void W25Q_Erase_Chip(void)
{
	uint8_t tData = 0x60;  											// Chip Erase

	write_enable();
	csLOW();
	SPI_Write(&tData, 1);
	csHIGH();
	write_disable();

	while(W25Q_ReadStatus(1)&0x01);									// Wait for BUSY to go low, TODO add timeout?
}

void W25Q_Erase_Sector(uint16_t numsector)
{
	uint8_t tData[6];
	uint32_t memAddr = numsector*FS_SECTOR_SIZE;					// Each sector contains 16 pages * 256 bytes

	write_enable();

	tData[0] = 0x20;  												// Erase sector
	tData[1] = (memAddr>>16)&0xFF;  								// MSB of the memory Address
	tData[2] = (memAddr>>8)&0xFF;
	tData[3] = (memAddr)&0xFF; 										// LSB of the memory Address

	csLOW();
	SPI_Write(tData, 4);
	csHIGH();

	while(W25Q_ReadStatus(1)&0x01);									// Check BUSY is low, if not wait, TODO add timeout?

	write_disable();
}

void Write_page(uint32_t page, uint16_t offset, uint32_t size, const uint8_t *data)
{
	uint8_t tData[266];
	uint32_t memAddr = (page*FS_PAGE_SIZE)+offset;
	uint32_t indx = 0;

	dprintf("W25Q_Write(page=%ld, offset=%d, memaddr=%ld) memAddr=%08lx\n",page,offset,size,memAddr);

	write_enable();

	tData[0] = 0x02;  												// block program
	tData[1] = (memAddr>>16)&0xFF;  								// MSB of the memory Address
	tData[2] = (memAddr>>8)&0xFF;
	tData[3] = (memAddr)&0xFF; 										// LSB of the memory Address
	indx = 4;

	memcpy(&tData[indx],data,size);

	csLOW();
	SPI_Write(tData, indx+size);
	csHIGH();

	while(W25Q_ReadStatus(1)&0x01);									// Check BUSY is low, if not wait, TODO add timeout?
	write_disable();
}

void W25Q_Write_block(uint32_t block, uint16_t offset, uint32_t size, const uint8_t *data)
{
	uint32_t startpage=((block*FS_SECTOR_SIZE)+offset)/FS_PAGE_SIZE;
	uint32_t bytesleft=size;
	uint32_t newoff=offset%256;
	uint32_t bufptr=0;

	dprintf("W25Q_Write_block(%ld,%d,%ld)  startpage=%ld newoff=%ld\n",block,offset,size,startpage,newoff);

	dprintf("First %ld,%04ld,%03ld ",startpage,newoff,(FS_PAGE_SIZE-newoff));
	Write_page(startpage, newoff, (FS_PAGE_SIZE-newoff), data);				// First block
	bufptr=(FS_PAGE_SIZE-newoff);
	bytesleft-=(FS_PAGE_SIZE-newoff);

	startpage++;
	while (bytesleft) {

		if (bytesleft>256) {
			dprintf("Page %ld,%04d,%03d ",startpage,0,FS_PAGE_SIZE);
			Write_page(startpage, 0, FS_PAGE_SIZE, &data[bufptr]);
			bytesleft-=FS_PAGE_SIZE;
			bufptr+=FS_PAGE_SIZE;
		} else {
			if (newoff) {
				dprintf("Last  %ld,%04d,%03ld ",startpage,0,newoff);
				Write_page(startpage, 0, newoff, &data[bufptr]);	// Last block
			} else {
				dprintf("Last  %ld,%04d,%03ld ",startpage,0,bytesleft);
				Write_page(startpage, 0, bytesleft, &data[bufptr]);
				bufptr+=bytesleft;
			}
			bytesleft=0;
		}
		startpage++;
	}
	dprintf("\n");
}


void delay_us(uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim1,0);  								// clear counter
	while (__HAL_TIM_GET_COUNTER(&htim1) < us);  					// wait
}
