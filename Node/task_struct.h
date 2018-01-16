#ifndef task_struct
#define task_struct

#include <stdint.h>

#define TASK_MAX_NAME_LENGTH 20
#define MAX_TASKS 10

#define SCHEDULE_QUEUE_LENGTH 500

struct Task {
	void (*functionPointer)(); ///< pointer to task function
	uint32_t repeatCount; ///< number of times left to run again - if 10000 or above, repeat indefinitely
	uint32_t repeatPeriod; ///< ticks between consecutive repeats
	char name[TASK_MAX_NAME_LENGTH+1]; ///< name of task which is used in logging task names
};

struct ScheduleQueueEntry {
	uint8_t taskIndex; ///< index of taskList which this queue entry represents
	int32_t relativeTime; ///< ticks left until task runs 
};

struct Task taskList[MAX_TASKS];
struct ScheduleQueueEntry scheduleQueue[SCHEDULE_QUEUE_LENGTH];

#endif
