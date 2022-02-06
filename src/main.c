#include <stdint.h>
#include "stm32f103x6.h"

#include "globalVariables.h"
#include "init.h"
#include "uart.h"
#include "mini_snprintf.h"
#include "timers.h"

enum boitierStatu{Normal, Manual, Bridge};
enum boitierStatu boitierStatu = Normal;

extern struct uartRxData uartRxData[2];
void main (void)
{
	clkSetup();
	gpioSetup();
	uartSetup();
	timersSettup();
	systickSetup();

	GPIOC->ODR |= GPIO_ODR_ODR13;

	while(1)
	{	
		char* buffToRead;
		unsigned int dataSize;
		if(buffToRead = uartBufferToRead(0, &dataSize))
		{
			uartBufferTreated(0);
		}
		//switch(boitierStatu)
		//{
		//	case Normal:
		//		break;
		//	case Manual:
		//		break;
		//	case Bridge:
		//		break;
		//}
	}

}

