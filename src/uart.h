#ifndef __UART_H__
#define __UART_H__

#define UART_RX_BUFF_SIZE 30

struct uartRxData {
	char buff[2][UART_RX_BUFF_SIZE+1];
	unsigned int dataSize[2];
	unsigned char buffToWriteNum;
	unsigned int buffWriteIndex;
	int buffToReadNum;
};
//Reade available data from *buffToRead (is set to NULL if no data is available)
//write buffToRead = NULL when done to free the buffer

void uartSetup();
void uart1InitiateSend(const char * str, unsigned int size);
void uart2InitiateSend(const char * str, unsigned int size);
char* uartBufferToRead(unsigned int const uartPort, unsigned int * const dataSize);
void uartBufferTreated(unsigned int const uartPort);

#define uart1RemainToSend() (uint32_t const volatile)(DMA1_Channel4->CNDTR & 0xFFFF)
#define uart2RemainToSend() (uint32_t const volatile)(DMA1_Channel7->CNDTR & 0xFFFF)

#endif
