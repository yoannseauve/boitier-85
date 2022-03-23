#ifndef __MEMORY_h__
#define __MEMORY_h__

#define EEPROM_MAX_ACCESS_ATTEMPTS 1000
#define DATA_SAVE_INTERVAL 600 /*1min*/

struct savedOperationData{
	unsigned char t0;	//low temperature of e85 additional starter under obdII format
	unsigned char t1;	//high temperature of e85 additional starter under obdII format
	unsigned char starterEnrichmentInjection;
	unsigned char standardEnrichmentInjection;
};

int saveState(struct savedOperationData data);
int loadState(struct savedOperationData* data);

#endif
