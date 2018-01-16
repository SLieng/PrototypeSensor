#include "task_settings.h"

#include "led.h"
#include "radio.h"
#include "uart.h"

struct Task toggleGreenTask = {
	.functionPointer = &toggleGreenLED,
	.repeatCount = 100,
	.repeatPeriod = 100000,
	.name = "Toggle Green LED",
};

struct Task toggleRedTask = {
	.functionPointer = &toggleRedLED,
	.repeatCount = 100,
	.repeatPeriod = 250000,
	.name = "Toggle Red LED",
};

struct Task advertiseNodeTask = {
	.functionPointer = &advertiseNode,
	.repeatCount = 10000,
	.repeatPeriod = ADVERTISE_PERIOD,
	.name = "Advertise Node",
};

struct Task commandRxTask = {
	.functionPointer = &commandRx,
	.repeatCount = 10000,
	.repeatPeriod = COMMAND_RX_PERIOD,
	.name = "Command Rx",
};

struct Task pingBaseStationTask = {
	.functionPointer = &pingBaseStation,
	.repeatCount = 10000,
	.repeatPeriod = ADVERTISE_PERIOD,
	.name = "Ping Base Station",
};

struct Task initUARTTask = {
	.functionPointer = &initUART,
	.repeatCount = 1,
	.repeatPeriod = 100000,
	.name = "Initiate UART",
};


struct Task writeUARTQueueTask = {
	.functionPointer = &writeUARTQueue,
	.repeatCount = 100000,
	.repeatPeriod = 100000,
	.name = "Write UART Queue",
};
