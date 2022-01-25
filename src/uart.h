#ifndef __UART_H__
#define __UART_H__

#define UART_RX_BUFF_SIZE 10

struct uartRxData {
	char buff[2][UART_RX_BUFF_SIZE];
	unsigned char buffToWriteNum;
	unsigned int buffWriteIndex;
	char *buffToRead;
};
//Reade available data from *buffToRead (is set to NULL if no data is available)
//write buffToRead = NULL when done to free the buffer

void uartSetup();
void uart1InitiateSend(char * str, unsigned int size);
#endif
