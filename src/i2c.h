#ifndef __I2C_h__
#define __I2C_h__

#define EEPROM_ADDR 0b10100000

void i2cSetup();
int memWrite(uint16_t memAddr, unsigned int nbBytes, unsigned char bytes[]);
int memRead(uint16_t memAddr, unsigned int nbBytes, unsigned char bytes[]);

#endif
