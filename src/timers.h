#ifndef __TIMERS_H__
#define __TIMERS_H__

#include <stddef.h>

void timersSettup();
void systickSetup();

struct injectionData{
	uint16_t injectionStartTime;
	enum {injectOff = 0, injectOn = 1} ECUState;
};

#endif
