#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include "Board.h"

#include <stdint.h>

#include <stdio.h>

#include <driverlib/uart.h>
#include <inc/hw_memmap.h>
#include <driverlib/sys_ctrl.h>

#define UART_WRITE_PERIOD 10000

#define UART_QUEUE_SIZE 2048
#define LEVEL_WARNING_RATIO 0.9

#define WRITE_CYCLE_DELAY 100

Char uartQueue[UART_QUEUE_SIZE];
uint32_t uartQueueHead = 0;
uint32_t uartQueueTail = 0;
uint8_t uartWarningShown = 0;

char * itoa (int value, char *result, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

uint32_t uartQueueTaken() {
	uint32_t spaceTaken;
	if(uartQueueHead >= uartQueueTail) {
		spaceTaken = uartQueueHead - uartQueueTail;
	} else {
		spaceTaken = UART_QUEUE_SIZE + uartQueueHead - uartQueueTail;
	}
	return spaceTaken;
}

void uartQueuePush(char ch) {
	uint32_t levelWarningQueue = (uint32_t) (UART_QUEUE_SIZE*LEVEL_WARNING_RATIO);
	uint32_t spaceTaken = uartQueueTaken();

	if(spaceTaken >= levelWarningQueue) {
		if(!uartWarningShown) {
			uartWarningShown = true;
			//Log Warning TODO
		}
	}
	uartQueue[uartQueueHead] = ch;
	uartQueueHead++;
	if(uartQueueHead == UART_QUEUE_SIZE) {
		uartQueueHead = 0;
	}
}

void uartQueuePushString(char *string) {
	while (*string) {
		uartQueuePush(*string);
		string++;
	}
}

void log(char *message) {
	uartQueuePushString(message);
}

void logWithTick(char *message) {
	uint32_t currentTick = Clock_getTicks();
	char strCurrentTick[11];
	itoa(currentTick, strCurrentTick, 10);
	log(strCurrentTick);
	char whiteSpace[] = "    ";
	log(whiteSpace);
	log(message);
}

void put(uint32_t ui32Base, uint8_t ui8Data) {
	HWREG(UART0_BASE + UART_O_CTL) |= UART_CTL_TXE;
	HWREG(UART0_BASE) = ui8Data;
	uint32_t j;
	for(j = 0;j<WRITE_CYCLE_DELAY;j++) {
		asm(" NOP");
	}
	while(HWREG(UART0_BASE + UART_O_FR) & 0x00000004);
	//while(HWREG(ui32Base + UART_O_FR) & UART_FR_TXFF) {
	//}
	//Send the char.
	
	//HWREG(ui32Base + UART_O_DR) = ui8Data;	
}


void writeUART(char* startAddress, uint32_t size) {
	uint32_t i;
	char* currentAddress = startAddress;
	for(i=0; i<size;i++) {

		put(UART0_BASE, 'a');
		uint32_t j;
		for(j=0; j<10000;j++) {
			asm(" NOP");
		}
		currentAddress++;
	}
}

void initUART() {
	uartQueueHead = 0;
	uartQueueTail = 0;

	UARTDisable(UART0_BASE);
	UARTFIFODisable(UART0_BASE);

	//UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
	UARTConfigSetExpClk(UART0_BASE, SysCtrlClockGet(), 9600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	UARTEnable(UART0_BASE);
	uint32_t j;
	for(j=0; j<100000;j++) {
		asm(" NOP");
	}
	//UARTFIFOEnable(UART0_BASE);
}

	

void writeUARTQueue() {
	if(uartQueueHead > uartQueueTail) {
		writeUART(&uartQueue[uartQueueTail], uartQueueHead - uartQueueTail);
	}
	if(uartQueueHead < uartQueueTail) {
		writeUART(&uartQueue[uartQueueTail], UART_QUEUE_SIZE - uartQueueTail);
		writeUART(&uartQueue[0], uartQueueHead);
	}
}
