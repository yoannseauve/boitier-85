#include <stdint.h>
#include "stm32f103x6.h"

#include "globalVariables.h"
#include "init.h"
#include "uart.h"
#include "mini_snprintf.h"
#include "timers.h"
#include "operationData.h"
#include "i2c.h"

#define BAT_SEND_BUFFER_SIZE 50
#define ELM_SEND_BUFFER_SIZE 20
#define UART_FRAME_TIMEOUT 50 //5s

enum boitierStatu{Normal, Manual, Bridge};
enum boitierStatu boitierStatu = Normal;

extern struct uartRxData uartRxData[2];

struct savedOperationData savedOperationData = {0, 0, 0, 0};
unsigned char coolantTemperatur = 60; //20°C
unsigned char additionalEnrichement = 0; //enrichement for starter

uint16_t lastDataSaveDate = 0;
uint16_t lastObdRequestDate = 0;

void initELM();

void main (void)
{
	clkSetup();
	gpioSetup();
	uartSetup();
	timersSettup();
	systickSetup();
	i2cSetup();

	GPIOC->ODR |= GPIO_ODR_ODR13;

	char *dataFromBt, *dataFromELM;
	unsigned int dataFromBtSize, dataFromELMSize;
	char dispBuffer[BAT_SEND_BUFFER_SIZE];
	char elmBuffer[ELM_SEND_BUFFER_SIZE];

	//!!! read flash memory !!!
	initELM();

	while(1)
	{	
		if(coolantTemperatur < savedOperationData.t0) //max starter	
			additionalEnrichement = savedOperationData.starterEnrichmentInjection;	
		else if (coolantTemperatur >= savedOperationData.t1) //no starter
			additionalEnrichement = 0;
		else //starter
			additionalEnrichement = ((uint32_t)(savedOperationData.t1 - coolantTemperatur) * (uint32_t)savedOperationData.starterEnrichmentInjection) / (savedOperationData.t1 - savedOperationData.t0);

		enrichmentInjection = (savedOperationData.standardEnrichmentInjection + additionalEnrichement) > 127? 127 : savedOperationData.standardEnrichmentInjection + additionalEnrichement; //compute enrichement

		dataFromBt = uartBufferToRead(1, &dataFromBtSize);
		dataFromELM = uartBufferToRead(0, &dataFromELMSize);

		//boitier control
		if(dataFromBt && mini_snscanf(dataFromBt, dataFromBtSize, "bat") == 3 )
		{
			if(mini_snscanf(dataFromBt, dataFromBtSize, "bat help") >= 4)
			{
				uart2InitiateSend("bat [cmd] to control boitier85\ncmd:\n", 36);
				uart2InitiateSend("normal : switch to normal mode\n", 31);
				uart2InitiateSend("manual : switch to manual mode and set enrichment by hande\n", 59);
				uart2InitiateSend("bridge : switch to bridge mode and communicate with ELM327\n", 59);
				uart2InitiateSend("info : display system infos\n", 28);
				uart2InitiateSend("set t0 XX : set starter t0 parameter to XX - 40°C\n", 51);
				uart2InitiateSend("set t1 XX : set starter t1 parameter to XX - 40°C\n", 51);
				uart2InitiateSend("set starter : set starter additional enrichment parameter to XX/127\n\n", 69);
			}
			if(mini_snscanf(dataFromBt, dataFromBtSize, "bat info") >= 4)
			{
				uart2InitiateSend(dispBuffer, mini_snprintf(dispBuffer, BAT_SEND_BUFFER_SIZE, "Enrichissement : %u/128\n", enrichmentInjection)); 
				while(uart2RemainToSend());	
				uart2InitiateSend(dispBuffer, mini_snprintf(dispBuffer, BAT_SEND_BUFFER_SIZE, "Temperature liquide : %d°C\n", coolantTemperatur-40)); 
				while(uart2RemainToSend());	
				uart2InitiateSend(dispBuffer, mini_snprintf(dispBuffer, BAT_SEND_BUFFER_SIZE, "Starter t0 parameter : %d°C\n", savedOperationData.t0-40)); 
				while(uart2RemainToSend());	
				uart2InitiateSend(dispBuffer, mini_snprintf(dispBuffer, BAT_SEND_BUFFER_SIZE, "Starter t1 parameter : %d°C\n", savedOperationData.t1-40)); 
				while(uart2RemainToSend());	
				uart2InitiateSend(dispBuffer, mini_snprintf(dispBuffer, BAT_SEND_BUFFER_SIZE, "Starter additional enrichment : %d/127\n\n", savedOperationData.starterEnrichmentInjection)); 
			}

			if(mini_snscanf(dataFromBt, dataFromBtSize, "bat set t0") >= 10)
			{
				mini_snscanf(dataFromBt, dataFromBtSize, "bat set t0 %u", &savedOperationData.t0);
			}

			if(mini_snscanf(dataFromBt, dataFromBtSize, "bat set t1") >= 10)
			{
				mini_snscanf(dataFromBt, dataFromBtSize, "bat set t1 %u", &savedOperationData.t1);
			}
			if(mini_snscanf(dataFromBt, dataFromBtSize, "bat set starter") >= 15)
			{
				mini_snscanf(dataFromBt, dataFromBtSize, "bat set starter %u", &savedOperationData.starterEnrichmentInjection);
			}
			uartBufferTreated(1);
			continue;
		}

		if (dataFromBt != NULL && (dataFromBt[0] == '\r' || dataFromBt[0] == '\n') && dataFromBtSize == 1) //simple newline
		{
			uart2InitiateSend(dataFromBt, dataFromBtSize);	//echo the newline
			uartBufferTreated(1);
			continue;//dataFromBt = uartBufferToRead(0, &dataFromBtSize);
		}

		//boitier behaviour
		static unsigned char requestedData=0;
		static unsigned char waintingForData=0;
		switch(boitierStatu)
		{
			case Normal:
				//!! update savedOperationData.standardEnrichmentInjection //
			case Manual:
				//!! get info from ECU, temperature and fuel trim 
				switch(requestedData)
				{
					case 0: //coolant temperature
						if(!waintingForData)
						{
							uart1InitiateSend("0105\r", 5);
							lastObdRequestDate = systicksCounter;	
							waintingForData = 1;
						}
						else
						{
							if(dataFromELM && mini_snscanf(dataFromELM, dataFromELMSize, "4105%u", &coolantTemperatur) >= 5 )
							{
								waintingForData = 0;
								requestedData++;
							}
							if (systicksCounter - lastObdRequestDate > UART_FRAME_TIMEOUT) //reset elm due to frame timeout 
							{
								waintingForData = 0;
								initELM();
							}
						}
						break;
					default:
						requestedData = 0;
				}	
				break;
			case Bridge: //act as a bridge between ELM an BT interface
				if(dataFromBt)
				{
					uart1InitiateSend(dataFromBt, dataFromBtSize);
					uartBufferTreated(1);
				}
				if(dataFromELM)
				{
					uart2InitiateSend(dataFromELM, dataFromELMSize);
					uartBufferTreated(0);
				}
				break;
		}
	}

}

void initELM()
{
	unsigned int timeout = 10; //timeout setting can be different during reset
	uint16_t transmissionDate;

	char *dataFromELM;
	unsigned int dataFromELMSize;
	char elmBuffer[ELM_SEND_BUFFER_SIZE];

	while(uartBufferToRead(0, &dataFromELMSize)) //clean data from ELB buffer
		uartBufferTreated(0);

	uart1InitiateSend(elmBuffer, mini_snprintf(elmBuffer, ELM_SEND_BUFFER_SIZE, "AT Z\r")); //reset
	transmissionDate = systicksCounter;
	while( (systicksCounter - transmissionDate) < timeout) // wait for ELM response 
	{
		dataFromELM = uartBufferToRead(0, &dataFromELMSize);
		if(dataFromELM && mini_snscanf(dataFromELM, dataFromELMSize, "OBDII  V1.5") == 11)
		{
			uartBufferTreated(0);
			break;
		}
	}

	while(uart2RemainToSend());	
	uart1InitiateSend(elmBuffer, mini_snprintf(elmBuffer, ELM_SEND_BUFFER_SIZE, "AT E0\r")); //echo OFF
	transmissionDate = systicksCounter;
	while( (systicksCounter - transmissionDate) < timeout) // wait for ELM response 
	{
		dataFromELM = uartBufferToRead(0, &dataFromELMSize);
		if(dataFromELM && mini_snscanf(dataFromELM, dataFromELMSize, "OK") == 2)
		{
			uartBufferTreated(0);
			break;
		}
	}

	while(uart2RemainToSend());	
	uart1InitiateSend(elmBuffer, mini_snprintf(elmBuffer, ELM_SEND_BUFFER_SIZE, "AT L0\r")); //Linefeed OFF
	transmissionDate = systicksCounter;
	while( (systicksCounter - transmissionDate) < timeout) // wait for ELM response 
	{
		dataFromELM = uartBufferToRead(0, &dataFromELMSize);
		if(dataFromELM && mini_snscanf(dataFromELM, dataFromELMSize, "OK") == 2)
		{
			uartBufferTreated(0);
			break;
		}
	}

	while(uart2RemainToSend());	
	uart1InitiateSend(elmBuffer, mini_snprintf(elmBuffer, ELM_SEND_BUFFER_SIZE, "ATAT0\r")); //adaptative timing OFF
	transmissionDate = systicksCounter;
	while( (systicksCounter - transmissionDate) < timeout) // wait for ELM response 
	{
		dataFromELM = uartBufferToRead(0, &dataFromELMSize);
		if(dataFromELM && mini_snscanf(dataFromELM, dataFromELMSize, "OK") == 2)
		{
			uartBufferTreated(1);
			break;
		}
	}

	while(uart2RemainToSend());	
	uart1InitiateSend(elmBuffer, mini_snprintf(elmBuffer, ELM_SEND_BUFFER_SIZE, "AT SP3\r")); //set protocol to 3 (ISO 9141-2) to be changed depanding of the vehicule, automatic protocol detection does not seem to work properly
	transmissionDate = systicksCounter;
	while( (systicksCounter - transmissionDate) < timeout) // wait for ELM response 
	{
		dataFromELM = uartBufferToRead(0, &dataFromELMSize);
		if(dataFromELM && mini_snscanf(dataFromELM, dataFromELMSize, "OK") == 2)
		{
			uartBufferTreated(0);
			break;
		}
	}
}
