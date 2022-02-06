#include "globalVariables.h"

unsigned char volatile enrichissementInjection = 100; // injection duration to be increased by enrichissementInjection * 1/128 of the initial value 
uint16_t volatile systicksCounter = 0;

