#include <stddef.h>
#include "stm32f103x6.h"
#include "timers.h"
#include "globalVariables.h"

//timer_1 -> output
//timer_3 -> input capture
//tim_3 clk slave of tim 1

struct injectionData injector[4]; //initializer at 0 (injectOff)

void timersSettup()
{
	//TIM1->CR2 = TIM_CR2_MMS_0; //TIM1 enable used as trigger output (TRGO) to start TIM3
	//TIM1->SMCR = TIM_SMCR_MSM; //enable TIM1 Master/Slave mode
	TIM1->PSC = 0x0070; //clk prescaler = 112 -> counter_clk periode ~ 1.527 µs for sys_clk = 74 MHz
	TIM1->ARR = 0xFFFF; //counter autoreload -> max count for upcopunting

	//TIM1->CCMR1 = TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_1;
	TIM1->CCMR1 = TIM_CCMR1_OC1M_1 //TI1 output deactiveted on output compare match
		| TIM_CCMR1_OC2M_1; //TI2 output deactiveted on output compare match
	TIM1->CCMR2 = TIM_CCMR2_OC3M_1 //TI3 output deactiveted on output compare match
		| TIM_CCMR2_OC4M_1; //TI4 output deactiveted on output compare match

	TIM1->CCER = TIM_CCER_CC1NE //TIM1_ch1_N output active
		| TIM_CCER_CC2NE //TIM1_ch2_N output active
		| TIM_CCER_CC3NE //TIM1_ch3_N output active
		| TIM_CCER_CC4E;  //TIM1_ch4 output active

	TIM1->BDTR = TIM_BDTR_MOE //TIM1 main output enable
		| TIM_BDTR_OSSR; //oututs are anabled with inactive state when inactive

	TIM3->PSC = 0x0070; //clk prescaler = 112 -> counter_clk periode ~ 1.527 µs for sys_clk = 74 MHz
	TIM3->ARR = 0xFFFF; //counter autoreload -> max count for upcopunting
	TIM3->SMCR = TIM_SMCR_SMS_1	//TIM3 in tiger mod (started by TIM1 enable)
		| TIM_SMCR_SMS_2;
	TIM3->CCMR1 = TIM_CCMR1_CC1S_0 //CCR1 linked to TI1
		| TIM_CCMR1_CC2S_0; //CCR2 linked to TI2
	//| TIM_CCMR1_IC1F_0	//set input capture 1 filter
	//| TIM_CCMR1_IC1F_1
	//| TIM_CCMR1_IC1F_2
	//| TIM_CCMR1_IC1F_3
	//| TIM_CCMR1_IC2F_0	//set input capture 2 filter
	//| TIM_CCMR1_IC2F_1
	//| TIM_CCMR1_IC2F_2
	//| TIM_CCMR1_IC2F_3
	TIM3->CCMR2 = TIM_CCMR2_CC3S_0 //CCR3 linked to TI3
		| TIM_CCMR2_CC4S_0; //CCR4 linked to TI4
	//| TIM_CCMR1_IC3F_0	//set input capture 3 filter
	//| TIM_CCMR1_IC3F_1
	//| TIM_CCMR1_IC3F_2
	//| TIM_CCMR1_IC3F_3
	//| TIM_CCMR1_IC4F_0	//set input capture 4 filter
	//| TIM_CCMR1_IC4F_1
	//| TIM_CCMR1_IC4F_2
	//| TIM_CCMR1_IC4F_3
	TIM3->CCER = TIM_CCER_CC1P //captur on TI1 falling edge
		| TIM_CCER_CC1E //TI1 capure enabled
		| TIM_CCER_CC2P //captur on TI2 falling edge
		| TIM_CCER_CC2E //TI2 capure enabled
		| TIM_CCER_CC3P //captur on TI3 falling edge
		| TIM_CCER_CC3E //TI3 capure enabled
		| TIM_CCER_CC4P //captur on TI4 falling edge
		| TIM_CCER_CC4E; //TI4 capure enabled

	TIM3->DIER = TIM_DIER_CC1IE //enable interrupt on TI1 capture
		| TIM_DIER_CC2IE //enable interrupt on TI2 capture
		| TIM_DIER_CC3IE //enable interrupt on TI3 capture
		| TIM_DIER_CC4IE; //enable interrupt on TI4 capture


	NVIC_SetPriority(TIM3_IRQn, 0x01); //TIM3 interrupt at high priority
	NVIC_EnableIRQ(TIM3_IRQn); //TIM3 interrupt enable

	TIM3->CR1 = TIM_CR1_CEN; //enable TIM3
	TIM1->CR1 = TIM_CR1_CEN; //enable TIM1
	
} 


void TIM3Interrupt()
{
	if (TIM3->SR & TIM_SR_CC1IF)	//capture occured in TI1
	{
		//!!!!!!!!!!!!!!!!! read TIM3->CCR1 register to clear the flag
		if(injector[0].ECUState == injectOff)
		{
			// force TIM1_CH1 output to active (start injection)
			TIM1->CCMR1 = (TIM1->CCMR1 & (~TIM_CCMR1_OC1M)) | TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_2;

			injector[0].injectionStartTime = TIM3->CCR1;
			TIM3->CCER &= ~TIM_CCER_CC1P; //TI1 caputre on rising edge
			injector[0].ECUState = injectOn;
		}
		else if (injector[0].ECUState == injectOn)
		{
			register uint16_t value = TIM3->CCR1 + enrichmentInjection * ((0x0000FFFF & (TIM3->CCR1 - injector[0].injectionStartTime)) >> 7);
			TIM1->CCR1 = value;

			uint16_t plop = TIM1->CCMR1;
			TIM1->CCMR1 = (plop & (~TIM_CCMR1_OC1M)) | TIM_CCMR1_OC1M_1;	//outup set inactive on match
			TIM3->CCER |= TIM_CCER_CC1P; //TI1 caputre on falling edge

			uint16_t CNT = TIM1->CNT;
			if((value - CNT) > (0xFFFF - 100) || value == CNT) // if match was missed during code execution
			{
				GPIOA->ODR ^= GPIO_ODR_ODR12;
				TIM1->CCMR1 = (plop & (~TIM_CCMR1_OC1M)) | TIM_CCMR1_OC1M_2;  //force output to inactive
			}
			injector[0].ECUState = injectOff;
		}
	}
	if (TIM3->SR & TIM_SR_CC2IF)	//capture occured in TI2
	{
		//!!!!!!!!!!!!!!!!! read TIM3->CCR2 register to clear the flag
	}
	if (TIM3->SR & TIM_SR_CC3IF)	//capture occured in TI3
	{
		//!!!!!!!!!!!!!!!!! read TIM3->CCR3 register to clear the flag
	}
	if (TIM3->SR & TIM_SR_CC4IF)	//capture occured in TI4
	{
		//!!!!!!!!!!!!!!!!! read TIM3->CCR4 register to clear the flag
	}
}

void systickSetup()
{
	SysTick->LOAD = (uint32_t)(0x6DDD00) - 1;                         /* set reload register, 100ms delay for Fcore=72MHz*/
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
		SysTick_CTRL_TICKINT_Msk |
		SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */
	NVIC_SetPriority (SysTick_IRQn, 0x02); /* set Priority for Systick Interrupt to medium*/
	NVIC_EnableIRQ(SysTick_IRQn); //Systick interrupt enable
}

void systickInterrupt()
{
	GPIOC->ODR ^= GPIO_ODR_ODR13;
	systicksCounter++;
}
