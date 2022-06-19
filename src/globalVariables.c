#include "globalVariables.h"

volatile unsigned char enrichmentInjection = 0; // injection duration to be increased by enrichmentInjection * 1/128 of the initial value 
volatile uint16_t systicksCounter = 0;
volatile unsigned char elmReadyToReceive = 1; // set when receiving '>' cleared when a command is sent to ELM

