#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include <driverlib/cpu.h>

#include "task_struct.h"
#include "task_settings.h"

#include "uart.h"
#include "led.h"

#define TICKS_BEFORE_EVENT 100

char NEW_LINE[2] = {0x0A, 0x00};

uint8_t numTasks;

uint8_t queueHead;
uint8_t queueTail;

uint8_t connected;

uint32_t prevTicks;

void startupCode() {
	initLED();
	initUART();
    //addTask(&initUARTTask, 1000);
	addTask(&toggleRedTask, 250000);
	//addTask(&advertiseNodeTask, 100000);
	addTask(&writeUARTQueueTask, 100000);
}

void addQueueEntry(struct ScheduleQueueEntry *queueEntry) {
	if(queueHead == SCHEDULE_QUEUE_LENGTH) {
		queueHead = 0;
	}
	if(queueHead < queueTail) {
		scheduleQueue[queueHead] = *queueEntry;
		queueHead++;
		return;
	}
	if(queueHead == queueTail) {
		scheduleQueue[queueHead] = *queueEntry;
		queueHead++;
		return;
	}

	uint8_t compareWith = queueTail+1;
	while(compareWith != queueHead) {
		if(scheduleQueue[compareWith].relativeTime > queueEntry->relativeTime) {
			struct ScheduleQueueEntry tempEntry = scheduleQueue[compareWith];
			scheduleQueue[compareWith] = *queueEntry;
			uint32_t i;
			for(i=queueHead;i>(compareWith+1);i--) {
				scheduleQueue[i] = scheduleQueue[i-1];
			}
			scheduleQueue[compareWith+1] = tempEntry;
			queueHead++;
			return;
		}
		compareWith++;
	}
	scheduleQueue[queueHead] = *queueEntry;
	queueHead++;
	return;
}

void addTask (struct Task *task, uint32_t timeUntilFirst) {
	taskList[numTasks] = *task;
	struct ScheduleQueueEntry queueEntry = {numTasks, timeUntilFirst};
	addQueueEntry(&queueEntry);	
	numTasks++;
}

void schedulerTask()
{
	numTasks = 0;
	queueHead = 0;
	queueTail = 0;

	connected = 0;

	initializeRadio();

	startupCode();

	prevTicks = Clock_getTicks();

    while (1) {
		// Get Next Task
		uint8_t taskIndex = scheduleQueue[queueTail].taskIndex;
		int32_t relativeTime = scheduleQueue[queueTail].relativeTime;
		
		int32_t numSleepTicks = relativeTime;

		void (*functionPointer)() = taskList[taskIndex].functionPointer;
		if(numSleepTicks < 0){
			//Error
			numSleepTicks = 0;
		} else {
			CPUdelay(numSleepTicks);
		}

		functionPointer();

		log("TASK ");
		logWithTick(taskList[taskIndex].name);
		log(&NEW_LINE);

		uint32_t newTicks = Clock_getTicks();
		uint32_t delta = newTicks - prevTicks;
		prevTicks = newTicks;

		uint32_t i;
		for(i=queueTail;i<queueHead;i++) {
			scheduleQueue[i].relativeTime = scheduleQueue[i].relativeTime - delta;
		}
		
		uint32_t repeatCount = taskList[taskIndex].repeatCount;
		repeatCount--;
		if(repeatCount) {
			struct ScheduleQueueEntry queueEntry = {taskIndex, taskList[taskIndex].repeatPeriod + scheduleQueue[queueTail].relativeTime};
			addQueueEntry(&queueEntry);
			taskList[taskIndex].repeatCount = repeatCount;
		}
		queueTail++;

    }
}

