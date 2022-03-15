#include <stddef.h>
#include "stm32f103x6.h"
#include "i2c.h"

void i2cSetup()
{
	I2C1->CR2 |= I2C_CR2_FREQ_5; //CLK = 32Mhz (Pclk1)
	I2C1->CCR = I2C_CCR_FS  //fast mode
		| I2C_CCR_DUTY // 9/16 duty cycle
		| 4<<I2C_CCR_CCR_Pos; // 320Khz
	I2C1->TRISE = 17<<I2C_TRISE_TRISE_Pos; //max clk rise time 500ns
	I2C1->CR1 |= I2C_CR1_ACK | I2C_CR1_PE;
}

static inline char i2cWriteAddrCheckFail()
{
	while (!(I2C1->SR1 & I2C_SR1_ADDR))
		if(I2C1->SR1 & I2C_SR1_AF)
		{
			I2C1->SR1 &= ~I2C_SR1_AF;
			I2C1->CR1 |= I2C_CR1_STOP;
			return 1;
		}
	return 0;
}

static inline char i2cWriteDataCheckFail()
{
	while (!(I2C1->SR1 & I2C_SR1_TXE))
		if(I2C1->SR1 & I2C_SR1_AF)
		{
			I2C1->SR1 &= ~I2C_SR1_AF;
			I2C1->CR1 |= I2C_CR1_STOP;
			return 1;
		}
	return 0;
}

static inline char i2cReadDataCheckFail()
{
	while (!(I2C1->SR1 & I2C_SR1_RXNE))
		if(I2C1->SR1 & I2C_SR1_AF)
		{
			I2C1->SR1 &= ~I2C_SR1_AF;
			I2C1->CR1 |= I2C_CR1_STOP;
			return 1;
		}
	return 0;
}

int memWrite(uint16_t memAddr, unsigned int nbBytes, unsigned char bytes[])
{
	unsigned int bytesCount = 0;

	//START
	I2C1->CR1 |= I2C_CR1_START;
	while (!(I2C1->SR1 & I2C_SR1_SB));

	//device ADDR
	I2C1->DR = EEPROM_ADDR;
	if (i2cWriteAddrCheckFail())
		return -1;
	if (!(I2C1->SR2 & I2C_SR2_TRA)) //reading SR2 mandatory befor data transmission
		return -1;

	//memory ADDR
	I2C1->DR = (memAddr >> 8) & 0xff;
	if (i2cWriteDataCheckFail())
		return -1;
	I2C1->DR = memAddr & 0xff;
	if (i2cWriteDataCheckFail())
		return -1;

	//data
	while (bytesCount < nbBytes)	
	{
		if (i2cWriteDataCheckFail())
			return -1;
		I2C1->DR = bytes[bytesCount++];
	}
	while (!(I2C1->SR1 & (I2C_SR1_TXE | I2C_SR1_AF)));
	I2C1->SR1 &= ~I2C_SR1_AF;
	I2C1->CR1 |= I2C_CR1_STOP;
	return 0;
}

int memRead(uint16_t memAddr, unsigned int nbBytes, unsigned char bytes[])
{
	if (nbBytes == 0)
		return 0;
	unsigned int bytesCount = 0;
	//enable data ACK during reception
	I2C1->CR1 |= I2C_CR1_ACK;

	//START
	I2C1->CR1 |= I2C_CR1_START;
	while (!(I2C1->SR1 & I2C_SR1_SB));

	//device ADDR
	I2C1->DR = EEPROM_ADDR;
	if (i2cWriteAddrCheckFail())
		return -1;
	if (!(I2C1->SR2 & I2C_SR2_TRA)) //reading SR2 mandatory before data transmission
		return -1;

	//memory ADDR
	I2C1->DR = (memAddr >> 8) & 0xff;
	if (i2cWriteDataCheckFail())
		return -1;
	I2C1->DR = memAddr & 0xff;
	if (i2cWriteDataCheckFail())
		return -1;

	//second START
	I2C1->CR1 |= I2C_CR1_START;
	while (!(I2C1->SR1 & I2C_SR1_SB));

	//device ADDR
	I2C1->DR = EEPROM_ADDR | 0x01; //R/W_ bit set to 1
	if (i2cWriteAddrCheckFail())
		return -1;
	if (nbBytes <= 1) //prepar stop if only one byte is to be read
		I2C1->CR1 &= ~I2C_CR1_ACK;
	if (I2C1->SR2 & I2C_SR2_TRA) //reading SR2 mandatory before data transmission
		return -1;

	//data
	while (bytesCount < nbBytes)	
	{
		if (bytesCount + 1 == nbBytes) //prepar to not ack and STOP before last byte is received
		{
			I2C1->CR1 &= ~I2C_CR1_ACK;
			I2C1->CR1 |= I2C_CR1_STOP;
		}
		while(!(I2C1->SR1 & I2C_SR1_RXNE));
		//I2C1->DR = bytes[bytesCount++];
		bytes[bytesCount++] = I2C1->DR;
	}
	return 0;
}
