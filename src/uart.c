#include <stddef.h>
#include "stm32f103x6.h"
#include "uart.h"

struct uartRxData uartRxData[2] = {{{"", ""}, {0, 0}, 0, 0, -1}, {{"", ""}, {0, 0}, 0, 0, -1}};

void uartSetup()
{
	//UART1
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

	//UART2
	USART2->BRR = 58 << USART_BRR_DIV_Mantissa_Pos | 9 << USART_BRR_DIV_Fraction_Pos; //for 38400 (38420) baud as long as Pclk1 = 36MHz
	USART2->CR1 = USART_CR1_UE //UART enable
		| USART_CR1_TE	//Transmitter enable
		| USART_CR1_RE  //Receive enable
		| USART_CR1_RXNEIE;	//Reveive interrupt enable
	USART2->CR3 = USART_CR3_DMAT;	//Enable DMA for transmitter

	DMA1_Channel7->CCR = DMA_CCR_MINC //Memory increment mode
		| DMA_CCR_DIR; //read from memory
	DMA1_Channel7->CPAR = (uint32_t) &USART2->DR;

	NVIC_SetPriority(USART2_IRQn, 0x03); //uart1 interrupt at low priority
	NVIC_EnableIRQ(USART2_IRQn); //uart1 interrupt enable
}

void uart1InitiateSend(const char * str, unsigned int size)
{
	while(DMA1_Channel4->CNDTR > 0);  //wait for previous DMA transfer to be done

	DMA1_Channel4->CCR &= ~DMA_CCR_EN; //channel disable
	DMA1_Channel4->CMAR = (uint32_t) str;
	DMA1_Channel4->CNDTR = size;
	DMA1_Channel4->CCR |= DMA_CCR_EN; //channel enable
}

void uart2InitiateSend(const char * str, unsigned int size)
{
	while(DMA1_Channel7->CNDTR > 0);  //wait for previous DMA transfer to be done

	DMA1_Channel7->CCR &= ~DMA_CCR_EN; //channel disable
	DMA1_Channel7->CMAR = (uint32_t) str;
	DMA1_Channel7->CNDTR = size;
	DMA1_Channel7->CCR |= DMA_CCR_EN; //channel enable
}

void uart1Interrupt()
{
	if(USART1->SR & USART_SR_RXNE_Msk)  //One byte was received
	{
		char data = USART1->DR;
		uartRxData[0].buff[uartRxData[0].buffToWriteNum][uartRxData[0].buffWriteIndex++] = data;
		if (uartRxData[0].buffWriteIndex >= UART_RX_BUFF_SIZE || data == '\r' || data == '\n')
		{
			if(uartRxData[0].buffWriteIndex == 1) //'\r' or '\n' is the first and only character, ignore the line
			{
				uartRxData[0].buffWriteIndex = 0;
				return;
			}
			uartRxData[0].buff[uartRxData[0].buffToWriteNum][uartRxData[0].buffWriteIndex] = '\0';
			uartRxData[0].buffToReadNum = uartRxData[0].buffToWriteNum;
			uartRxData[0].dataSize[uartRxData[0].buffToWriteNum] = uartRxData[0].buffWriteIndex;
			uartRxData[0].buffToWriteNum = uartRxData[0].buffToWriteNum ? 0 : 1;
			uartRxData[0].buffWriteIndex = 0;
		}	
	}
}

void uart2Interrupt()
{
	if(USART2->SR & USART_SR_RXNE_Msk)  //One byte was received
	{
		char data = USART2->DR;
		uartRxData[1].buff[uartRxData[1].buffToWriteNum][uartRxData[1].buffWriteIndex++] = data;
		if (uartRxData[1].buffWriteIndex >= UART_RX_BUFF_SIZE || data == '\r' || data == '\n' || data == '>')
		{

			if(uartRxData[1].buffWriteIndex == 1) //'\r' or '\n' is the first and only character, ignore the line
			{
				uartRxData[1].buffWriteIndex = 0;
				return;
			}
			uartRxData[1].buff[uartRxData[1].buffToWriteNum][uartRxData[1].buffWriteIndex] = '\0';
			uartRxData[1].buffToReadNum = uartRxData[1].buffToWriteNum;
			uartRxData[1].dataSize[uartRxData[1].buffToWriteNum] = uartRxData[1].buffWriteIndex;
			uartRxData[1].buffToWriteNum = uartRxData[1].buffToWriteNum ? 0 : 1;
			uartRxData[1].buffWriteIndex = 0;
		}	
	}
}

char* uartBufferToRead(unsigned int const uartPort, unsigned int * const dataSize)
{
	if (uartPort > 2)
		return NULL;
	int buffToReadNum = uartRxData[uartPort].buffToReadNum;

	if(buffToReadNum == -1)
	{
		if (dataSize != NULL)
			*dataSize = 0;
		return NULL;
	}

	if (dataSize != NULL)
		*dataSize = uartRxData[uartPort].dataSize[buffToReadNum];

	return  uartRxData[uartPort].buff[buffToReadNum];
}

void uartBufferTreated(unsigned int const uartPort)
{
	if (uartPort > 2)
		return ;
	if (uartRxData[uartPort].buffToReadNum == uartRxData[uartPort].buffToWriteNum)	//the other buff is already full
	{
		uartRxData[uartPort].buffToReadNum = uartRxData[uartPort].buffToWriteNum ? 0 : 1;
	}
	else
		uartRxData[uartPort].buffToReadNum = -1;
}
