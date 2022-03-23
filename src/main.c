#include <stdint.h>
#include "stm32f103x6.h"

#include "globalVariables.h"
#include "init.h"
#include "uart.h"
#include "mini_snprintf.h"
#include "timers.h"
#include "memory.h"
#include "i2c.h"

#define BAT_SEND_BUFFER_SIZE 50
#define ELM_SEND_BUFFER_SIZE 20
#define UART_FRAME_TIMEOUT 50 //5s
#define ELM_RESET_TIMEOUT 50 //5s
#define LONG_TERM_FUEL_TRIM_TOLERANCE 10
#define SHORT_TERM_FUEL_TRIM_TOLERANCE 20
#define FUEL_ENRICHMENT_CORRECTION_DELAY 50 //5s

enum boitierStatu{Normal, Manual, Bridge};
enum boitierStatu boitierStatu = Normal;

extern struct uartRxData uartRxData[2];

struct savedOperationData savedOperationData = {0, 0, 0, 0};
unsigned char coolantTemperatur = 60; //20°C
unsigned char longTermFuelTrim = 0x80; //neutal
unsigned char shortTermFuelTrim = 0x80; //neutal
unsigned char additionalEnrichement = 0; //enrichment for starter

uint16_t lastDataSaveDate = 0;
uint16_t lastObdRequestDate = 0;
uint16_t lastEnrichmentUpdateDate = 0;

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
	//char elmBuffer[ELM_SEND_BUFFER_SIZE];
	uint16_t dataSaveDate = systicksCounter;

	static unsigned char requestedData=0;
	static unsigned char waitingForData=0;


	loadState(&savedOperationData);
	requestedData = 101; //initELM

	while(1)
	{	
		if(boitierStatu !=  Manual) //comput enrichmentInjection only if not manually set
		{
			if(coolantTemperatur < savedOperationData.t0) //max starter	
				additionalEnrichement = savedOperationData.starterEnrichmentInjection;	
			else if (coolantTemperatur >= savedOperationData.t1) //no starter
				additionalEnrichement = 0;
			else //starter
				additionalEnrichement = ((uint32_t)(savedOperationData.t1 - coolantTemperatur) * (uint32_t)savedOperationData.starterEnrichmentInjection) / (savedOperationData.t1 - savedOperationData.t0);

			enrichmentInjection = (savedOperationData.standardEnrichmentInjection + additionalEnrichement) > 127? 127 : savedOperationData.standardEnrichmentInjection + additionalEnrichement; //compute enrichment
		}

		if(systicksCounter - dataSaveDate > DATA_SAVE_INTERVAL)
		{
			saveState(savedOperationData);
			//if( ! saveState(savedOperationData))
			//	uart2InitiateSend("MEM save\n", 9);
			//else
			//	uart2InitiateSend("MEM nosave\n", 11);
			dataSaveDate = systicksCounter;
		}

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
				uart2InitiateSend("set starter : set starter additional enrichment parameter to XX/127\n", 68);
				uart2InitiateSend("set enrichment : set enrichment factor to XX/127 (only used in manual mode)\n\n", 77);
			}
			if(mini_snscanf(dataFromBt, dataFromBtSize, "bat info") >= 4)
			{

				switch(boitierStatu)
				{
					case Normal:
						uart2InitiateSend("Mode : Normal\n", 14);
						break;
					case Manual:
						uart2InitiateSend("Mode : Manual\n", 14);
						break;
					case Bridge:
						uart2InitiateSend("Mode : Bridge\n", 14);
						break;
				}
				uart2InitiateSend(dispBuffer, mini_snprintf(dispBuffer, BAT_SEND_BUFFER_SIZE, "Enrichissement : %u/127\n", enrichmentInjection)); 
				while(uart2RemainToSend());	
				uart2InitiateSend(dispBuffer, mini_snprintf(dispBuffer, BAT_SEND_BUFFER_SIZE, "Temperature liquide : %d°C\n", coolantTemperatur-40)); 
				while(uart2RemainToSend());	
				uart2InitiateSend(dispBuffer, mini_snprintf(dispBuffer, BAT_SEND_BUFFER_SIZE, "Starter t0 parameter : %d°C\n", savedOperationData.t0-40)); 
				while(uart2RemainToSend());	
				uart2InitiateSend(dispBuffer, mini_snprintf(dispBuffer, BAT_SEND_BUFFER_SIZE, "Starter t1 parameter : %d°C\n", savedOperationData.t1-40)); 
				while(uart2RemainToSend());	
				uart2InitiateSend(dispBuffer, mini_snprintf(dispBuffer, BAT_SEND_BUFFER_SIZE, "Starter additional enrichment : %d/127\n", savedOperationData.starterEnrichmentInjection)); 
				while(uart2RemainToSend());	
				uart2InitiateSend(dispBuffer, mini_snprintf(dispBuffer, BAT_SEND_BUFFER_SIZE, "ELM long term fuel trim : %d \n", longTermFuelTrim)); 
				while(uart2RemainToSend());	
				uart2InitiateSend(dispBuffer, mini_snprintf(dispBuffer, BAT_SEND_BUFFER_SIZE, "ELM short term fuel trim : %d \n\n", shortTermFuelTrim)); 
			}

			if(mini_snscanf(dataFromBt, dataFromBtSize, "bat set t0") >= 10)
			{
				unsigned int tmp_int;
				mini_snscanf(dataFromBt, dataFromBtSize, "bat set t0 %u", &tmp_int);
				savedOperationData.t0 = (unsigned char) tmp_int;
				saveState(savedOperationData);
				dataSaveDate = systicksCounter;
				uart2InitiateSend("OK\n", 3);
			}

			if(mini_snscanf(dataFromBt, dataFromBtSize, "bat set t1") >= 10)
			{
				unsigned int tmp_int;
				mini_snscanf(dataFromBt, dataFromBtSize, "bat set t1 %u", &tmp_int);
				savedOperationData.t1 = (unsigned char) tmp_int;
				saveState(savedOperationData);
				dataSaveDate = systicksCounter;
				uart2InitiateSend("OK\n", 3);

			}
			if(mini_snscanf(dataFromBt, dataFromBtSize, "bat set starter") >= 15)
			{
				unsigned int tmp_int;
				mini_snscanf(dataFromBt, dataFromBtSize, "bat set starter %u", &tmp_int);
				savedOperationData.starterEnrichmentInjection = (unsigned char) tmp_int;
				saveState(savedOperationData);
				dataSaveDate = systicksCounter;
				uart2InitiateSend("OK\n", 3);

			}
			if(mini_snscanf(dataFromBt, dataFromBtSize, "bat set enrichment") >= 18)
			{
				unsigned int tmp_int;
				mini_snscanf(dataFromBt, dataFromBtSize, "bat set enrichment %u", &tmp_int);
				savedOperationData.standardEnrichmentInjection = (unsigned char) tmp_int;
				enrichmentInjection = (unsigned char) tmp_int;
				saveState(savedOperationData);
				dataSaveDate = systicksCounter;
				uart2InitiateSend("OK\n", 3);

			}
			if(mini_snscanf(dataFromBt, dataFromBtSize, "bat normal") >= 10)
			{
				boitierStatu = Normal;
				uart2InitiateSend("Mode : Normal\n", 14);
			}
			if(mini_snscanf(dataFromBt, dataFromBtSize, "bat manual") >= 10)
			{
				boitierStatu = Manual;
				uart2InitiateSend("Mode : Manual\n", 14);
			}
			if(mini_snscanf(dataFromBt, dataFromBtSize, "bat bridge") >= 10)
			{
				boitierStatu = Bridge;
				uart2InitiateSend("Mode : Bridge\n", 14);
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
		switch(boitierStatu)
		{
			case Normal:
				//!! update savedOperationData.standardEnrichmentInjection //
			case Manual:
				//!! get info from ECU, temperature and fuel trim 
				switch(requestedData)
				{
					case 0: //coolant temperature
						if(!waitingForData)
						{
							uart1InitiateSend("0105\r", 5);
							lastObdRequestDate = systicksCounter;	
							waitingForData = 1;
						}
						else
						{
							if(dataFromELM && mini_snscanf(dataFromELM, dataFromELMSize, "4105%u", &coolantTemperatur) >= 5 )
							{
								waitingForData = 0;
								requestedData++;
							}
							if (systicksCounter - lastObdRequestDate > UART_FRAME_TIMEOUT) //reset elm due to frame timeout 
							{
								waitingForData = 0;
								requestedData = 101;//initELM
							}
						}
						break;
					
					case 1: //long term fuel trim
						if(!waitingForData)
						{
							uart1InitiateSend("0107\r", 5);
							lastObdRequestDate = systicksCounter;	
							waitingForData = 1;
						}
						else
						{
							if(dataFromELM && mini_snscanf(dataFromELM, dataFromELMSize, "4107%u", &longTermFuelTrim) >= 5 )
							{
								waitingForData = 0;
								requestedData++;
							}
							if (systicksCounter - lastObdRequestDate > UART_FRAME_TIMEOUT) //reset elm due to frame timeout 
							{
								waitingForData = 0;
								requestedData = 101;//initELM
							}
						}
						break;

					case 2: //short term fuel trim
						if(!waitingForData)
						{
							uart1InitiateSend("0106\r", 5);
							lastObdRequestDate = systicksCounter;	
							waitingForData = 1;
						}
						else
						{
							if(dataFromELM && mini_snscanf(dataFromELM, dataFromELMSize, "4106%u", &shortTermFuelTrim) >= 5 )
							{
								waitingForData = 0;
								requestedData = 0; //last data to request
								//update fuel enrichement
								if (systicksCounter - lastEnrichmentUpdateDate > FUEL_ENRICHMENT_CORRECTION_DELAY && boitierStatu == Normal)
								{
									if (longTermFuelTrim > 0x80 + LONG_TERM_FUEL_TRIM_TOLERANCE) //too lean
									{
										if(shortTermFuelTrim > 0x80 - SHORT_TERM_FUEL_TRIM_TOLERANCE) //no obvious previous correction
											if(savedOperationData.standardEnrichmentInjection < 127)
												savedOperationData.standardEnrichmentInjection ++;
									}
									if (longTermFuelTrim < 0x80 - LONG_TERM_FUEL_TRIM_TOLERANCE) //too rich
									{
										if(shortTermFuelTrim < 0x80 + SHORT_TERM_FUEL_TRIM_TOLERANCE) //no obvious previous correction
											if(savedOperationData.standardEnrichmentInjection>0)
												savedOperationData.standardEnrichmentInjection --;
									}
									lastEnrichmentUpdateDate = systicksCounter;
								}
							}
							if (systicksCounter - lastObdRequestDate > UART_FRAME_TIMEOUT) //reset elm due to frame timeout 
							{
								waitingForData = 0;
								requestedData = 101;//initELM
							}
						}
						break;

					case 101: //Reset First State
						while(uartBufferToRead(0, &dataFromELMSize)) //clean data from ELM buffer
							uartBufferTreated(0);
						requestedData = 102;
						break;

					case 102: //send reset
						uart1InitiateSend("AT Z\r", 5); //reset
						lastObdRequestDate = systicksCounter;
						requestedData = 103;
						break;

					case 103: //wait for answer
						if(systicksCounter - lastObdRequestDate > ELM_RESET_TIMEOUT)
							requestedData = 101; //start new reset cycle

						dataFromELM = uartBufferToRead(0, &dataFromELMSize);
						if(dataFromELM)
						{
							if(mini_snscanf(dataFromELM, dataFromELMSize, "OBDII  v1.5") == 11)
							{
								requestedData = 104;	
							}
							uartBufferTreated(0);
						}
						break;

					case 104: //set echo off
						uart1InitiateSend("AT E0\r", 6);
						lastObdRequestDate = systicksCounter;
						requestedData = 105;
						break;

					case 105: //wait for answer
						if(systicksCounter - lastObdRequestDate > ELM_RESET_TIMEOUT)
							requestedData = 101; //start new reset cycle

						dataFromELM = uartBufferToRead(0, &dataFromELMSize);
						if(dataFromELM)
						{

							if(mini_snscanf(dataFromELM, dataFromELMSize, "OK") == 2)
							{
								requestedData = 106;	
							}
							uartBufferTreated(0);
						}
						break;

					case 106:  //set Linefeed OFF
						uart1InitiateSend("AT L0\r", 6);
						lastObdRequestDate = systicksCounter;
						requestedData = 107;
						break;

					case 107: //wait for answer
						if(systicksCounter - lastObdRequestDate > ELM_RESET_TIMEOUT)
							requestedData = 101; //start new reset cycle

						dataFromELM = uartBufferToRead(0, &dataFromELMSize);
						if(dataFromELM)
						{
							if(mini_snscanf(dataFromELM, dataFromELMSize, "OK") == 2)
							{
								requestedData = 108;	
							}
							uartBufferTreated(0);
						}
						break;

					case 108: //set adaptative timing OFF
						uart1InitiateSend("ATAT0\r", 6);
						lastObdRequestDate = systicksCounter;
						requestedData = 109;
						break;

					case 109: //wait for answer
						if(systicksCounter - lastObdRequestDate > ELM_RESET_TIMEOUT)
							requestedData = 101; //start new reset cycle

						dataFromELM = uartBufferToRead(0, &dataFromELMSize);
						if(dataFromELM)
						{
							if(mini_snscanf(dataFromELM, dataFromELMSize, "OK") == 2)
							{
								requestedData = 110;	
							}
							uartBufferTreated(0);
						}
						break;

					case 110: //set protocol to ISO 9141-2
						uart1InitiateSend("AT SP3\r", 7);  //set protocol
						lastObdRequestDate = systicksCounter;
						requestedData = 111;
						break;

					case 111: //wait for answer
						if(systicksCounter - lastObdRequestDate > ELM_RESET_TIMEOUT)
							requestedData = 101; //start new reset cycle

						dataFromELM = uartBufferToRead(0, &dataFromELMSize);
						if(dataFromELM)
						{
							if(mini_snscanf(dataFromELM, dataFromELMSize, "OK") == 2)
							{
								requestedData = 0;	
							}
							uartBufferTreated(0);
						}
						break;

					default:
						requestedData = 0;
				}	
				break;

			case Bridge: //act as a bridge between ELM an BT interface
				if(dataFromBt)
				{
					while(uart1RemainToSend());	
					uart1InitiateSend(dataFromBt, dataFromBtSize);
					uartBufferTreated(1);
				}
				if(dataFromELM)
				{
					while(uart2RemainToSend());	
					uart2InitiateSend(dataFromELM, dataFromELMSize);
					uartBufferTreated(0);
				}
				break;
		}
	}

}

//void initELM()
//{
//	unsigned int timeout = 10; //timeout setting can be different during reset
//	uint16_t transmissionDate;
//
//	char *dataFromELM;
//	unsigned int dataFromELMSize;
//	char elmBuffer[ELM_SEND_BUFFER_SIZE];
//
//	while(uartBufferToRead(0, &dataFromELMSize)) //clean data from ELM buffer
//		uartBufferTreated(0);
//
//	uart1InitiateSend(elmBuffer, mini_snprintf(elmBuffer, ELM_SEND_BUFFER_SIZE, "AT Z\r")); //reset
//	transmissionDate = systicksCounter;
//	while( (unsigned int)(systicksCounter - transmissionDate) < timeout) // wait for ELM response 
//	{
//		dataFromELM = uartBufferToRead(0, &dataFromELMSize);
//
//		if(dataFromELM)
//		{
//			if(mini_snscanf(dataFromELM, dataFromELMSize, "OBDII  v1.5") == 11)
//			{
//				uartBufferTreated(0);
//				break;
//			}
//			uartBufferTreated(0);
//		}
//	}
//
//	while(uart2RemainToSend());	
//	uart1InitiateSend(elmBuffer, mini_snprintf(elmBuffer, ELM_SEND_BUFFER_SIZE, "AT E0\r")); //echo OFF
//	transmissionDate = systicksCounter;
//	while( (unsigned int)(systicksCounter - transmissionDate) < timeout) // wait for ELM response 
//	{
//		dataFromELM = uartBufferToRead(0, &dataFromELMSize);
//		if(dataFromELM && mini_snscanf(dataFromELM, dataFromELMSize, "OK") == 2)
//		{
//			uartBufferTreated(0);
//			break;
//		}
//	}
//
//	while(uart2RemainToSend());	
//	uart1InitiateSend(elmBuffer, mini_snprintf(elmBuffer, ELM_SEND_BUFFER_SIZE, "AT L0\r")); //Linefeed OFF
//	transmissionDate = systicksCounter;
//	while( (unsigned int)(systicksCounter - transmissionDate) < timeout) // wait for ELM response 
//	{
//		dataFromELM = uartBufferToRead(0, &dataFromELMSize);
//		if(dataFromELM && mini_snscanf(dataFromELM, dataFromELMSize, "OK") == 2)
//		{
//			uartBufferTreated(0);
//			break;
//		}
//	}
//
//	while(uart2RemainToSend());	
//	uart0InitiateSend(elmBuffer, mini_snprintf(elmBuffer, ELM_SEND_BUFFER_SIZE, "ATAT0\r")); //adaptative timing OFF
//	transmissionDate = systicksCounter;
//	while( (unsigned int)(systicksCounter - transmissionDate) < timeout) // wait for ELM response 
//	{
//		dataFromELM = uartBufferToRead(0, &dataFromELMSize);
//		if(dataFromELM && mini_snscanf(dataFromELM, dataFromELMSize, "OK") == 2)
//		{
//			uartBufferTreated(1);
//			break;
//		}
//	}
//
//	while(uart2RemainToSend());	
//	uart1InitiateSend(elmBuffer, mini_snprintf(elmBuffer, ELM_SEND_BUFFER_SIZE, "AT SP3\r")); //set protocol to 3 (ISO 9141-2) to be changed depanding of the vehicule, automatic protocol detection does not seem to work properly
//	transmissionDate = systicksCounter;
//	while( (unsigned int)(systicksCounter - transmissionDate) < timeout) // wait for ELM response 
//	{
//		dataFromELM = uartBufferToRead(0, &dataFromELMSize);
//		if(dataFromELM && mini_snscanf(dataFromELM, dataFromELMSize, "OK") == 2)
//		{
//			uartBufferTreated(0);
//			break;
//		}
//	}
//}
