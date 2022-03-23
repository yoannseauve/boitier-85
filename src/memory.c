#include <stddef.h>
#include "memory.h"
#include "i2c.h"

char dispBuffer[50];

int memReadAttempt(uint16_t memAddr, unsigned int nbBytes, unsigned char bytes[])
{
    int memAccessError, memAccessAttempts;
    memAccessAttempts=0;
    do
        memAccessError = memRead(memAddr, nbBytes, bytes);
    while(memAccessError == -1 && memAccessAttempts++ < EEPROM_MAX_ACCESS_ATTEMPTS);
    if (memAccessError == -1)
    {
        return -1;
    }
    return 0;
}

int memWriteAttempt(uint16_t memAddr, unsigned int nbBytes, unsigned char bytes[])
{
    int memAccessError, memAccessAttempts;
    memAccessAttempts=0;
    do
        memAccessError = memWrite(memAddr, nbBytes, bytes);
    while(memAccessError == -1 && memAccessAttempts++ < EEPROM_MAX_ACCESS_ATTEMPTS);
    if (memAccessError == -1)
    {
        return -1;
    }
    return 0;
}

int saveState(struct savedOperationData data)
{
    unsigned char page[64];
    unsigned int rowToWrite;
    unsigned int dataLocation = 0;//location inside the page

	struct savedOperationData dataCopy;
	for ( unsigned int i = 0; i< sizeof(struct savedOperationData); i++)
		((unsigned char*) &dataCopy)[i] = ((unsigned char*) &data)[i];

	dataCopy.standardEnrichmentInjection |= 0x80; //force "standardEnrichmentInjection" to 1 => flag used tu identify used data slot

	if ( memReadAttempt(0, 64, page)) //first page contains a bitfield, each 1 bit represent a full page of data
		return -1;
	rowToWrite = 512;
	for(int i = 0; i < 64; i++)
	{
		if (page[i]==0xFF)
			continue;
		for(int j = 0; j<8; j++)
			if(! (page[i] & (1<<j)))
			{
				rowToWrite = 1 + 8 * i + j;
				break;
			}
		break;
	}

	if(rowToWrite >= 512) //EEPROM is FULL
	{
		//clean page 1 and write data
		for (unsigned int i = 0; i<64; i++)
		{
			if(i < sizeof(struct savedOperationData))
				page[i] = ((unsigned char*)(&dataCopy))[i];
			else
				page[i]=0;
		}
		if( memWriteAttempt(64, 64, page) )
			return -1;
		//empty page 0
		for (int i = 0; i<64; i++)
		{
			page[i]=0;
		}
		if( memWriteAttempt(0, 64, page))
			return -1;
		return 0;
	}

	//search fo free space in the page
	//MSB of "standardEnrichmentInjection" used to indicated occupied data location;
	if( memReadAttempt(rowToWrite << 6, 64, page))
		return -1;

	dataLocation = sizeof(struct savedOperationData) * ((64/sizeof(struct savedOperationData))-1);
	for(unsigned int i = 0; i <= 64 - sizeof(struct savedOperationData); i += sizeof(struct savedOperationData))
	{
		if (! (((struct savedOperationData*) (&(page[i])))->standardEnrichmentInjection & 0x80)) //free data location
		{
			dataLocation = i;
			break;
		}
	}

	//if last available slot
	if (dataLocation > 64 - 2*sizeof(struct savedOperationData))
	{
		//empty next page
		if (rowToWrite < 511)
		{
			for (int i = 0; i<64; i++)
			{
				page[i]=0;
			}
			if( memWriteAttempt((rowToWrite+1)<<6, 64, page))
				return -1;
		}
		//write data
		if( memWriteAttempt((rowToWrite<<6) | dataLocation, sizeof(struct savedOperationData), (unsigned char*) (&dataCopy)))
			return -1;
		//update page 1
		unsigned char byte;
		if( memReadAttempt((rowToWrite-1)/8, 1, &byte))
			return -1;
		byte |= 1<<(rowToWrite-1) % 8;
		if( memWriteAttempt((rowToWrite-1)/8, 1, &byte))
			return -1;
	}
	else
		if( memWriteAttempt((rowToWrite<<6) | dataLocation, sizeof(struct savedOperationData), (unsigned char*) (&dataCopy)))
			return -1;
	return 0;
}

int loadState(struct savedOperationData* data)
{
	unsigned char page[64];
	unsigned int rowToRead;
	unsigned int dataLocation = 0;//location inside the page

	if( memReadAttempt(0, 64, page)) //first page contains a bitfield, each 1 bit represent a full page of data
		return -1;

	rowToRead = 512;
	for(int i = 0; i < 64; i++)
	{
		if (page[i]==0xFF)
			continue;
		for(int j = 0; j<8; j++)
			if(! (page[i] & (1<<j)))
			{
				rowToRead = 1 + 8 * i + j;
				break;
			}
		break;
	}

	if(rowToRead >= 512) //EEPROM is FULL
	{
		rowToRead = 511;
		dataLocation = ((64/sizeof(struct savedOperationData))-1)*sizeof(struct savedOperationData);
	}
	else
	{
		if( memReadAttempt(rowToRead << 6, 64, page))
			return -1;
		if((((struct savedOperationData*) (&(page[0])))->standardEnrichmentInjection & 0x80) == 0) //page is empty
		{
			if(rowToRead == 1) //EEPROM is empty
				return -1;
			rowToRead--;
			if( memReadAttempt(rowToRead << 6, 64, page))
				return -1;
		}
		//search for last data slot
		for(unsigned int i = 0; i <= 64 - sizeof(struct savedOperationData); i+= sizeof(struct savedOperationData))
		{
			if(((struct savedOperationData*) (&page[i]))->standardEnrichmentInjection & 0x80) //slot is used
			{
				dataLocation = i;
				continue;
			}
			break;
		}
	}
	if( memReadAttempt((rowToRead << 6) + dataLocation, sizeof(struct savedOperationData), (unsigned char*)data))
		return -1;
	data->standardEnrichmentInjection &= ~ 0x80; //removed "standardEnrichmentInjection" MSB which is used as flag for memory storage
	return 0;
}

