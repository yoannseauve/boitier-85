#include "stm32f103x6.h"

void clkSetup()
{
	RCC->CR |= RCC_CR_HSEON; //start HSE

	RCC->CFGR = RCC_CFGR_PLLSRC /*HSE as PLL input*/
		| RCC_CFGR_PLLMULL9 //PLL set to x9
		| RCC_CFGR_PPRE1_DIV2; //APB1 prescaler 2

	RCC->CR |= RCC_CR_PLLON;    //start PLL
	FLASH->ACR |= FLASH_ACR_LATENCY_2 | FLASH_ACR_PRFTBE; //enable flash prefetch
	while ( !(RCC->CR & RCC_CR_PLLRDY) || !(RCC->CR & RCC_CR_HSERDY));  //wait for HSE and PLL to be ready
	RCC->CFGR |= RCC_CFGR_SW_1; //switch SYSCLK on PLL
	//RCC->CR &= ~RCC_CR_HSION; //stop HSI DON'T, it breaks programming function

	RCC->APB2ENR |= RCC_APB2ENR_USART1EN	//enable uart1 clk
		| RCC_APB2ENR_IOPAEN    //enable GPIO port A clk
		| RCC_APB2ENR_IOPBEN    //enable GPIO port B clk
		| RCC_APB2ENR_IOPCEN	//enable GPIO port C clk
		| RCC_APB2ENR_TIM1EN;   //enable TIM1 clk

	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;	//enable TIM3 clk
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;	//enable alternatFunctions (petipheral remap) clk

	RCC->AHBENR |= RCC_AHBENR_DMA1EN;	//enable DMA1 clk

}

void gpioSetup()
{
	AFIO->MAPR |= AFIO_MAPR_TIM3_REMAP_1;	//remap TIM3_CH1 & TIM3_CH2

	GPIOA->CRL &=  ~(GPIO_CRL_CNF2 | GPIO_CRL_MODE2)
		& ~(GPIO_CRL_CNF3 | GPIO_CRL_MODE3);
	GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9)
		& ~(GPIO_CRH_CNF10 | GPIO_CRH_MODE10)
		& ~GPIO_CRH_CNF11;
	GPIOA->CRL |=  GPIO_CRL_CNF2_1 | GPIO_CRL_MODE2_0 //PA2 (uart2 tx) output_max 10 MHz push-pull
		| GPIO_CRL_CNF3_0;  //PA3 (uart2 rx) input pull-up/pull-down
	GPIOA->CRH |= GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9_0  //PA9 (uart1 tx) output_max 10 MHz push-pull
		| GPIO_CRH_CNF10_0  //PA10 (uart1 rx) input pull-up/pull-down
		| GPIO_CRH_CNF11_1 | GPIO_CRH_MODE11_0 | GPIO_CRH_MODE11_1;  //PA11 (TIM1_CH4) output push-pull 50MHz
	GPIOA->ODR |= GPIO_ODR_ODR10 //PA10 (uart1 rx) pull-up
		| GPIO_ODR_ODR3; //PA3 (uart2 rx) pull-up

	GPIOB->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_CNF1 | GPIO_CRL_CNF4 | GPIO_CRL_CNF5);
	GPIOB->ODR |= GPIO_ODR_ODR0 //PB0 (TIM3_CH3) high (pull-up)
		| GPIO_ODR_ODR1 //PB1 (TIM3_CH4) high (pull-up)
		| GPIO_ODR_ODR4 //PB4 (TIM3_CH1_remap) high (pull-up)
		| GPIO_ODR_ODR5;  //PB5 (TIM3_CH2_remap) high (pull-up)
	GPIOB->CRL |= GPIO_CRL_CNF0_1  //PB0 (TIM3_CH3) input pull-up/pull-down
		| GPIO_CRL_CNF1_1  //PB1 (TIM3_CH4) input pull-up/pull-down
		| GPIO_CRL_CNF4_1  //PB4 (TIM3_CH1_remap) input pull-up/pull-down
		| GPIO_CRL_CNF5_1; //PB5 (TIM3_CH2_remap) input pull-up/pull-down

	GPIOB->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_CNF14 | GPIO_CRH_CNF15);
	GPIOB->CRH |= GPIO_CRH_CNF13_1 | GPIO_CRH_MODE13_0 | GPIO_CRH_MODE13_1 //PB13 (TIM1_CH1N) output push-pull 
		| GPIO_CRH_CNF14_1 | GPIO_CRH_MODE14_0 | GPIO_CRH_MODE14_1	 //PB14 (TIM1_CH2N) output push-pull 50MHz
		| GPIO_CRH_CNF15_1 | GPIO_CRH_MODE15_0 | GPIO_CRH_MODE15_1;	 //PB15 (TIM1_CH3N) output push-pull 50MHz

	GPIOC->CRH |= GPIO_CRH_MODE13_1;	//PC13 output open-drain
}
