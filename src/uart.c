#include <stddef.h>
#include "stm32f103x6.h"
#include "uart.h"

volatile struct uartRxData uartRxData[2] = {{"", "", 0, 0, NULL}, {"", "", 0, 0, NULL}};

void uartSetup()
{

	USART1->BRR = 117 << USART_BRR_DIV_Mantissa_Pos | 3 << USART_BRR_DIV_Fraction_Pos; //for 38400 baud as long as Pclk2 = 72MHz
	USART1->CR1 = USART_CR1_UE //UART enable
	| USART_CR1_TE	//Transmitter enable
	| USART_CR1_RE  //Receive enable
	| USART_CR1_RXNEIE;	//Reveive interrupt enable
	USART1->CR3 = USART_CR3_DMAT;	//Enable DMA for transmitter

	DMA1_Channel4->CCR = DMA_CCR_MINC //Memory increment mode
	| DMA_CCR_DIR; //read from memory
	DMA1_Channel4->CPAR = (uint32_t) &USART1->DR;
	
	NVIC_SetPriority(USART1_IRQn, 0x03); //uart1 interrupt at low priority
	NVIC_EnableIRQ(USART1_IRQn); //uart1 interrupt enable
}

void uart1InitiateSend(char * str, unsigned int size)
{
	while(DMA1_Channel4->CNDTR > 0);  //wait for previous DMA transfer to be done

	DMA1_Channel4->CCR &= ~DMA_CCR_EN; //channel disable
	DMA1_Channel4->CMAR = (uint32_t) str;
	DMA1_Channel4->CNDTR = size;
	DMA1_Channel4->CCR |= DMA_CCR_EN; //channel enable
}

void uart1Interrupt()
{
	if(USART1->SR & USART_SR_RXNE_Msk)  //One byte was received
	{
		uartRxData[0].buff[0][0] = USART1->DR;
	}
}
