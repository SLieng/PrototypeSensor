/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
// #include <ti/drivers/Watchdog.h>

#include <driverlib/prcm.h>
#include <driverlib/sys_ctrl.h>
#include <driverlib/gpio.h>
#include <driverlib/setup.h>

/* Board Header files */
#include "Board.h"

// Include Own Files
#include "scheduler.h"
#include "uart.h"

#define TASKSTACKSIZE   512

#define SCHEDULER_PRIORITY 1

Task_Struct schedulerTaskStruct;
Char schedulerStack[TASKSTACKSIZE];

void mcu_boot() {
    // Trim device
    SetupTrimDevice(); //Was trimDevice();

    // Power on necessary power domains
    PRCMPowerDomainOn(PRCM_DOMAIN_VIMS | PRCM_DOMAIN_SYSBUS | PRCM_DOMAIN_CPU | PRCM_DOMAIN_SERIAL | PRCM_DOMAIN_PERIPH);
    while (PRCMPowerDomainStatus(PRCM_DOMAIN_VIMS | PRCM_DOMAIN_SYSBUS | PRCM_DOMAIN_CPU | PRCM_DOMAIN_SERIAL | PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON) { }
    PRCMLoadSet();
    while(!PRCMLoadGet()) { }

    // Enable clock to necessary power domains
    PRCMDomainEnable(PRCM_DOMAIN_VIMS | PRCM_DOMAIN_SYSBUS | PRCM_DOMAIN_CPU | PRCM_DOMAIN_SERIAL | PRCM_DOMAIN_PERIPH);
    PRCMLoadSet();
    while(!PRCMLoadGet()) { }

    // Use HF crystal as clock source
    OSCClockSourceSet(OSC_SRC_CLK_MF | OSC_SRC_CLK_HF, OSC_XOSC_HF);
    uint32_t i;
    for ( i = OSCClockSourceGet(OSC_SRC_CLK_HF); i != OSC_XOSC_HF; i = OSCClockSourceGet(OSC_SRC_CLK_HF)) {
        OSCHfSourceSwitch();
    }

    // Use LF crystal as clock source
    OSCClockSourceSet(OSC_SRC_CLK_LF, OSC_XOSC_LF);

    PRCMLoadSet();
    while(!PRCMLoadGet()) { }
}

int main(void)
{
	//mcu_boot();
	SysCtrlPowerEverything();

    // Enable clock to necessary power domains
    PRCMDomainEnable(PRCM_DOMAIN_VIMS | PRCM_DOMAIN_SYSBUS | PRCM_DOMAIN_CPU | PRCM_DOMAIN_SERIAL | PRCM_DOMAIN_PERIPH);
    PRCMLoadSet();
    while(!PRCMLoadGet()) { }

    // Use HF crystal as clock source
    OSCClockSourceSet(OSC_SRC_CLK_MF | OSC_SRC_CLK_HF, OSC_XOSC_HF);
    uint32_t i;
    for ( i = OSCClockSourceGet(OSC_SRC_CLK_HF); i != OSC_XOSC_HF; i = OSCClockSourceGet(OSC_SRC_CLK_HF)) {
        OSCHfSourceSwitch();
    }

    // Use LF crystal as clock source
    OSCClockSourceSet(OSC_SRC_CLK_LF, OSC_XOSC_LF);

    PRCMLoadSet();
    while(!PRCMLoadGet()) { }

	//SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

	schedulerTask();

    //Task_Params schedulerTaskParams;

    /* Call board init functions */
    //Board_initGeneral();
    // Board_initI2C();
    // Board_initSPI();
    //Board_initUART();
    // Board_initWatchdog();

    /* Construct Scheduler Task thread */
    //Task_Params_init(&schedulerTaskParams);
    //schedulerTaskParams.arg0 = 1000000 / Clock_tickPeriod;
    //schedulerTaskParams.stackSize = TASKSTACKSIZE;
    //schedulerTaskParams.stack = &schedulerStack;
	//schedulerTaskParams.priority = SCHEDULER_PRIORITY;
    //Task_construct(&schedulerTaskStruct, (Task_FuncPtr)schedulerTask, &schedulerTaskParams, NULL);

    /* Open LED pins */
    //ledPinHandle = PIN_open(&ledPinState, ledPinTable);
    //if(!ledPinHandle) {
        //System_abort("Error initializing board LED pins\n");
    //}

    /* Start BIOS */
    //BIOS_start();

    return (0);
}
