#include <stdint.h>
#include "stm32f103x6.h"

#include "globalVariables.h"
#include "init.h"
#include "uart.h"
#include "mini_snprintf.h"
#include "timers.h"

void main (void)
{
	clkSetup();
	gpioSetup();
	uartSetup();
	timersSettup();
	systickSetup();

	GPIOC->ODR |= GPIO_ODR_ODR13;

	GPIOA->CRH &= ~(GPIO_CRH_CNF12 | GPIO_CRH_MODE12);
	GPIOA->CRH |= GPIO_CRH_MODE12_0 | GPIO_CRH_MODE12_1;  //PA9 (uart1 tx) output_max 10 MHz push-pull

	while(1)
	{	
		//GPIOC->ODR ^= GPIO_ODR_ODR13;
		for (int i = 0; i < 500000; i++); // arbitrary delay

		char* buffToRead;
		if(buffToRead = uartBufferToRead(0))
		{
			unsigned int i = 0;
			while (buffToRead[i] && i <= UART_RX_BUFF_SIZE)
				i++;
			uart1InitiateSend(buffToRead, i);
			uartBufferTreated(0);
		}
	}

}
