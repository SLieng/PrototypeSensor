#include "uart.h"

#include <driverlib/prcm.h>
#include <driverlib/sys_ctrl.h>
#include <driverlib/gpio.h>
#include <driverlib/setup.h>

#include <driverlib/rf_mailbox.h>
#include <driverlib/rf_common_cmd.h>
#include <driverlib/rf_ble_mailbox.h>
#include <driverlib/rf_ble_cmd.h>
#include <driverlib/rf_prop_mailbox.h>
#include <driverlib/rfc.h>

#include <rf_patches/rf_patch_cpe_genfsk.h>
#include <rf_patches/rf_patch_rfe_genfsk.h>
#include <rf_patches/rf_patch_mce_genfsk.h>

#include <ti/sysbios/knl/Task.h>

#include "smartrf_settings/smartrf_settings.h"
#include "RFQueue.h"

#include "Board.h" //LED

#include "task_settings.h"

#define RF_CMD0                                0x0607

extern PIN_Handle ledPinHandle;
extern uint8_t connected;

/* Packet RX Configuration */
#define DATA_ENTRY_HEADER_SIZE 8  /* Constant header size of a Generic Data Entry */
#define MAX_LENGTH             4 /* Max length byte the radio will accept */
#define NUM_DATA_ENTRIES       2  /* NOTE: Only two data entries supported at the moment */
#define NUM_APPENDED_BYTES     2

#define PAYLOAD_LENGTH      4
#define PACKET_INTERVAL     (uint32_t)(4000000*0.5f*0.2) /* Set packet interval to 500ms */
#define FIVE     (uint32_t)(4000000*0.5f*0.2) /* Set packet interval to 500ms */

uint32_t time;
static uint8_t packet[PAYLOAD_LENGTH];

#if defined(__TI_COMPILER_VERSION__)
    #pragma DATA_ALIGN (rxDataEntryBuffer, 4);
        static uint8_t rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                 MAX_LENGTH,
                                                                 NUM_APPENDED_BYTES)];
#endif

/* Receive dataQueue for RF Core to fill in data */
static dataQueue_t dataQueue;
static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t packetLength;
static uint8_t* packetDataPointer;

// Turns on the RF code and all necessary power domains
static void rf_core_on() {
    // Disable RF core in case it was enabled
    PRCMPowerDomainOff(PRCM_DOMAIN_RFCORE);
    PRCMLoadSet();
    while (!PRCMLoadGet()) { }

    //Mode 3 and 5 works, 5 is bluetooth + prop mode, 3 is prop mode only
    HWREG(PRCM_BASE + PRCM_O_RFCMODESEL) = PRCM_RFCMODESEL_CURR_MODE3;

    // Power on RF core
    PRCMPowerDomainOn(PRCM_DOMAIN_RFCORE);
    while (PRCMPowerDomainStatus(PRCM_DOMAIN_RFCORE) != PRCM_DOMAIN_POWER_ON) { }
    PRCMLoadSet();
    while (!PRCMLoadGet()) { }

    // Enable RF core clock
    PRCMDomainEnable(PRCM_DOMAIN_RFCORE);
    PRCMLoadSet();
    while (!PRCMLoadGet()) { }

    //Turn Acknowledgement off
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;

    //Turn on Additional Clocks
    HWREG(RFC_DBELL_BASE+RFC_DBELL_O_CMDR) = CMDR_DIR_CMD_2BYTE(RF_CMD0, RFC_PWR_PWMCLKEN_MDMRAM | RFC_PWR_PWMCLKEN_RFERAM);

    // Enablde RF clocks
    HWREG(RFC_PWR_NONBUF_BASE + RFC_PWR_O_PWMCLKEN) = 0x7FF;

    while(!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG));
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;

    rf_patch_cpe_genfsk();

    rf_patch_mce_genfsk();
    rf_patch_rfe_genfsk();

    //Turn off Additional Clocks
    RFCDoorbellSendTo(CMDR_DIR_CMD_2BYTE(RF_CMD0, 0));

    while( HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDSTA) == CMDSTA_Pending);
        if( HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDSTA) != CMDSTA_Done )
            while (1);

    /* Start RAT timer */
    uint32_t result = RFCDoorbellSendTo(CMDR_DIR_CMD(CMD_START_RAT));

    // Wait for it...
    PRCMLoadSet();
    while (!PRCMLoadGet()) { }
}

void initializeRadio() {
    rf_core_on();

    //Initialize bus request
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;
    HWREG(RFC_DBELL_BASE+RFC_DBELL_O_CMDR) = CMDR_DIR_CMD_1BYTE(CMD_BUS_REQUEST, 1);
    while(!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG));
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;

    if(RFQueue_defineQueue(&dataQueue,
                                rxDataEntryBuffer,
                                sizeof(rxDataEntryBuffer),
                                NUM_DATA_ENTRIES,
                                MAX_LENGTH + NUM_APPENDED_BYTES))
        {
            /* Failed to allocate space for all data entries */
            while(1);
        }

    RF_cmdPropRadioDivSetup.startTrigger.triggerType = TRIG_NOW;
    RF_cmdPropRadioDivSetup.startTrigger.pastTrig = 1;
    RF_cmdPropRadioDivSetup.startTime = 0;

    RF_cmdFs.startTrigger.triggerType = TRIG_NOW;
    RF_cmdFs.startTrigger.pastTrig = 1;
    RF_cmdFs.startTime = 0;

    uint32_t result;

    result = RFCDoorbellSendTo((uint32_t) &RF_cmdPropRadioDivSetup);
    uint32_t i;
    for(i=0;i<1000000;i++) {
        asm(" NOP");
    }

    result = RFCDoorbellSendTo((uint32_t) &RF_cmdFs);
    for(i=0;i<1000000;i++) {
        asm(" NOP");
    }

    RF_cmdPropTx.pktLen = PAYLOAD_LENGTH;
    RF_cmdPropTx.pPkt = packet;
    RF_cmdPropTx.startTrigger.triggerType = TRIG_NOW;
    RF_cmdPropTx.startTrigger.pastTrig = 1;
    RF_cmdPropTx.startTime = 0;

    packet[0] = 20;
    packet[1] = 20;
    packet[2] = 30;
    packet[3] = 40;

    /* Modify CMD_PROP_RX command for application needs */
    RF_cmdPropRx.pQueue = &dataQueue;           /* Set the Data Entity queue for received data */
    RF_cmdPropRx.rxConf.bAutoFlushIgnored = 1;  /* Discard ignored packets from Rx queue */
    RF_cmdPropRx.rxConf.bAutoFlushCrcErr = 1;   /* Discard packets with CRC error from Rx queue */
    RF_cmdPropRx.maxPktLen = MAX_LENGTH;        /* Implement packet length filtering to avoid PROP_ERROR_RXBUF */
    RF_cmdPropRx.pktConf.bRepeatOk = 0;
    RF_cmdPropRx.pktConf.bRepeatNok = 1;

    RF_cmdPropRx.endTrigger.triggerType = TRIG_REL_SUBMIT;
    RF_cmdPropRx.endTime = DEFAULT_TIMEOUT;

}

void transmitPacket() {
    log("TRANSMIT PACKET START\n");
    OSCClockSourceSet(OSC_SRC_CLK_MF | OSC_SRC_CLK_HF, OSC_XOSC_HF);
    uint32_t result = RFCDoorbellSendTo((uint32_t) &RF_cmdPropTx);
    uint32_t i;
    for(i=0;i<100;i++) {
        asm(" NOP");
	}
	while(RF_cmdPropTx.status != 0x3400);
	log("TRANSMIT PACKET END\n");
}

void receivePacket() {
    log("RECEIVE PACKET START\n");
    OSCClockSourceSet(OSC_SRC_CLK_MF | OSC_SRC_CLK_HF, OSC_XOSC_HF);
	uint32_t result;
	uint32_t i;
    result = RFCDoorbellSendTo((uint32_t) &RF_cmdPropRx);
    for(i=0;i<100;i++) {
        asm(" NOP");
    }
    while(RF_cmdPropRx.status == 0x0002);
	log("RECEIVE PACKET END\n");
}

void advertiseNode() {
	if(connected == 1) {
		return 0;
	}
    log("RADIO START\n");
	transmitPacket();
	receivePacket();
    if(RF_cmdPropRx.status == 0x3401) {
        //Timeouts
    } else {
		connected = 1;
		addTask(&pingBaseStationTask, 100000);
        log("Connected\n");
    }
    log("RADIO END\n");
    PIN_setOutputValue(ledPinHandle, Board_LED0, !PIN_getOutputValue(Board_LED0));
    return 0;
}

void pingBaseStation() {
	log("PING START\n");
	transmitPacket();
	return 0;
}

void commandRx() {
	uint32_t tickBeforeRx = Clock_getTicks();
    PIN_setOutputValue(ledPinHandle, Board_LED0, !PIN_getOutputValue(Board_LED0));
	receivePacket();		
    if(RF_cmdPropRx.status == 0x3401) {
        //Timeouts
    } else {
		transmitPacket();
        log("Found Advertising Node");
	}
    PIN_setOutputValue(ledPinHandle, Board_LED0, !PIN_getOutputValue(Board_LED0));
}
