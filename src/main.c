#include <stdint.h>
#include "stm32f103x6.h"

#include "init.h"
#include "uart.h"
#include "mini_snprintf.h"
#include "timers.h"

uint32_t debug[20];
volatile uint16_t debug_count[3];
volatile uint16_t debug_count_flag = 0;

unsigned char enrichissementInjection = 100; // injection duration to be increased by enrichissementInjection * 1/128 of the initial value 

void main (void)
{
	clkSetup();
	gpioSetup();
	uartSetup();
	timersSettup();

	GPIOC->ODR |= GPIO_ODR_ODR13;

GPIOA->CRH &= ~(GPIO_CRH_CNF12 | GPIO_CRH_MODE12);
	GPIOA->CRH |= GPIO_CRH_MODE12_0 | GPIO_CRH_MODE12_1;  //PA9 (uart1 tx) output_max 10 MHz push-pull

	GPIOC->ODR ^= GPIO_ODR_ODR13;
	while(1)
	{	
		GPIOC->ODR ^= GPIO_ODR_ODR13;

		for (int i = 0; i < 40000; i++); // arbitrary delay
		char str1[10];
		char str2[10];
		int size;
		if (debug_count_flag)
		{
			size=mini_snprintf(str1, 15, "V%u", debug_count[0]);
			uart1InitiateSend(str1, size);
			size=mini_snprintf(str2, 15, " C%u", debug_count[1]);
			uart1InitiateSend(str2, size);
			size=mini_snprintf(str1, 15, " S%u", debug_count[2]);
			uart1InitiateSend(str1, size);

			debug_count_flag = 0;
		}
		for (int i = 0; i < 500000; i++); // arbitrary delay
		GPIOC->ODR ^= GPIO_ODR_ODR13;
		for (int i = 0; i < 50000; i++); // arbitrary delay

	}

}
