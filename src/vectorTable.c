#include <stddef.h>

void _reset_handler(void);
void uart1Interrupt(void);
void uart2Interrupt(void);
void TIM3Interrupt(void);
void systickInterrupt(void);

void (*__interupt__vectors__[]) (void) __attribute__((section(".vector_table"))) =
{
	_reset_handler,		//Reset 		Reset								0x0000_0004
	_reset_handler,		//NMI 		Non maskable interrupt. The RCC Clock Security System (CSS) is linked to the NMI vector.							0x0000_0008
	_reset_handler,		//HardFault 		All class of fault							0x0000_000c
	_reset_handler,		//MemManage 		Memory management							0x0000_0010
	_reset_handler,		//BusFault 		Prefetch fault, memory access fault							0x0000_0014
	_reset_handler,		//UsageFault 		Undefined instruction or illegal state							0x0000_0018
	NULL,			  	//Reserved							0x0000_001c
	NULL,			  	//Reserved							0x0000_0020
	NULL,			  	//Reserved							0x0000_0024
	NULL,			  	//Reserved							0x0000_0028
	_reset_handler,		//SVCall 		System service call via SWI instruction							0x0000_002c
	_reset_handler,		//Debug 		Monitor Debug Monitor							0x0000_0030
	NULL,  				//Reserved							0x0000_0034
	_reset_handler,		//PendSV 		Pendable request for system service							0x0000_0038
	systickInterrupt,		//SysTick 		System tick timer							0x0000_003c
	_reset_handler,		//WWDG 		Window watchdog interrupt							0x0000_0040
	_reset_handler,		//PVD 		PVD through EXTI Line detection interrupt							0x0000_0044
	_reset_handler,		//TAMPER 		Tamper interrupt							0x0000_0048
	_reset_handler,		//RTC 		RTC global interrupt							0x0000_004c
	_reset_handler,		//FLASH 		Flash global interrupt							0x0000_0050
	_reset_handler,		//RCC 		RCC global interrupt							0x0000_0054
	_reset_handler,		//EXTI0 		EXTI Line0 interrupt							0x0000_0058
	_reset_handler,		//EXTI1 		EXTI Line1 interrupt							0x0000_005c
	_reset_handler,		//EXTI2 		EXTI Line2 interrupt							0x0000_0060
	_reset_handler,		//EXTI3 		EXTI Line3 interrupt							0x0000_0064
	_reset_handler,		//EXTI4 		EXTI Line4 interrupt							0x0000_0068
	_reset_handler,		//DMA1_Channel1 		DMA1 Channel1 global interrupt							0x0000_006c
	_reset_handler,		//DMA1_Channel2 		DMA1 Channel2 global interrupt							0x0000_0070
	_reset_handler,		//DMA1_Channel3 		DMA1 Channel3 global interrupt							0x0000_0074
	_reset_handler,		//DMA1_Channel4 		DMA1 Channel4 global interrupt							0x0000_0078
	_reset_handler,		//DMA1_Channel5 		DMA1 Channel5 global interrupt							0x0000_007c
	_reset_handler,		//DMA1_Channel6 		DMA1 Channel6 global interrupt							0x0000_0080
	_reset_handler,		//DMA1_Channel7 		DMA1 Channel7 global interrupt							0x0000_0084
	_reset_handler,		//ADC1_2 		ADC1 and ADC2 global interrupt							0x0000_0088
	_reset_handler,		//USB_HP_CAN_ 		TX USB High Priority or CAN TX interrupts							0x0000_008c
	_reset_handler,		//USB_LP_CAN_ 		RX0 USB Low Priority or CAN RX0 interrupts							0x0000_0090
	_reset_handler,		//CAN_RX1 		CAN RX1 interrupt							0x0000_0094
	_reset_handler,		//CAN_SCE 		CAN SCE interrupt							0x0000_0098
	_reset_handler,		//EXTI9_5 		EXTI Line[9:5] interrupts							0x0000_009c
	_reset_handler,		//TIM1_BRK 		TIM1 Break interrupt							0x0000_00a0
	_reset_handler,		//TIM1_UP 		TIM1 Update interrupt							0x0000_00a4
	_reset_handler,		//TIM1_TRG_COM 		TIM1 Trigger and Commutation interrupts							0x0000_00a8
	_reset_handler,		//TIM1_CC 		TIM1 Capture Compare interrupt							0x0000_00ac
	_reset_handler,		//TIM2 		TIM2 global interrupt							0x0000_00b0
	TIM3Interrupt,		//TIM3 		TIM3 global interrupt							0x0000_00b4
	_reset_handler,		//TIM4 		TIM4 global interrupt							0x0000_00b8
	_reset_handler,		//I2C1_EV 		I2C1 event interrupt							0x0000_00bc
	_reset_handler,		//I2C1_ER 		I2C1 error interrupt							0x0000_00c0
	_reset_handler,		//I2C2_EV 		I2C2 event interrupt							0x0000_00c4
	_reset_handler,		//I2C2_ER 		I2C2 error interrupt							0x0000_00c8
	_reset_handler,		//SPI1 		SPI1 global interrupt							0x0000_00cc
	_reset_handler,		//SPI2 		SPI2 global interrupt							0x0000_00d0
	uart1Interrupt,		//USART1 		USART1 global interrupt							0x0000_00d4
	uart2Interrupt,		//USART2 		USART2 global interrupt							0x0000_00d8
	_reset_handler,		//USART3 		USART3 global interrupt							0x0000_00dc
	_reset_handler,		//EXTI15_10 		EXTI Line[15:10] interrupts							0x0000_00e0
	_reset_handler,		//RTCAlarm 		RTC alarm through EXTI line interrupt							0x0000_00e4
	_reset_handler,		//USBWakeup 		USB wakeup from suspend through EXTI line interrupt							0x0000_00e8
	_reset_handler,		//TIM8_BRK 		TIM8 Break interrupt							0x0000_00ec
	_reset_handler,		//TIM8_UP 		TIM8 Update interrupt							0x0000_00f0
	_reset_handler,		//TIM8_TRG_COM 		TIM8 Trigger and Commutation interrupts							0x0000_00f4
	_reset_handler,		//TIM8_CC 		TIM8 Capture Compare interrupt							0x0000_00f8
	_reset_handler,		//ADC3 		ADC3 global interrupt							0x0000_00fc
	_reset_handler,		//FSMC 		FSMC global interrupt							0x0000_0100
	_reset_handler,		//SDIO 		SDIO global interrupt							0x0000_0104
	_reset_handler,		//TIM5 		TIM5 global interrupt							0x0000_0108
	_reset_handler,		//SPI3 		SPI3 global interrupt							0x0000_010c
	_reset_handler,		//UART4 		UART4 global interrupt							0x0000_0110
	_reset_handler,		//UART5 		UART5 global interrupt							0x0000_0114
	_reset_handler,		//TIM6 		TIM6 global interrupt							0x0000_0118
	_reset_handler,		//TIM7 		TIM7 global interrupt							0x0000_011c
	_reset_handler,		//DMA2_Channel1 		DMA2 Channel1 global interrupt							0x0000_0120
	_reset_handler,		//DMA2_Channel2 		DMA2 Channel2 global interrupt							0x0000_0124
	_reset_handler,		//DMA2_Channel3 		DMA2 Channel3 global interrupt							0x0000_0128
	_reset_handler		//DMA2_Channel4_5 		DMA2 Channel4 and DMA2 Channel5 global interrupts							0x0000_012c
};

