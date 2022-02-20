#include "globalVariables.h"

unsigned char volatile enrichmentInjection = 0; // injection duration to be increased by enrichmentInjection * 1/128 of the initial value 
uint16_t volatile systicksCounter = 0;

