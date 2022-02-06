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
void uart1InitiateSend(char * str, unsigned int size);
void uart2InitiateSend(char * str, unsigned int size);
char* uartBufferToRead(unsigned int uartPort, unsigned int * dataSize);
void uartBufferTreated(unsigned int uartPort);

#endif
