#ifndef __OPERATIONDATA_H__
#define __OPERATIONDATA_H__

struct savedOperationData{
	unsigned char t0;	//low temperature of e85 additional starter under obdII format
	unsigned char t1;	//high temperature of e85 additional starter under obdII format
	unsigned char starterEnrichmentInjection;
	unsigned char standardEnrichmentInjection;
};

#endif
