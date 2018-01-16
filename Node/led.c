#include "Board.h"

#include "led.h"

#include <driverlib/gpio.h>
#include <driverlib/ioc.h>

void initLED() {
	IOCPinTypeGpioOutput(Board_LED0);
	IOCPinTypeGpioOutput(Board_LED1);
}

void toggleGreenLED() {
	GPIO_toggleDio(Board_LED0);
}

void toggleRedLED() {
	GPIO_toggleDio(Board_LED1);
}
