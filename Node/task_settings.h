/** @file task_settings.h 
 *  @brief A list of all Tasks and their default parameters. Also includes settings for tasks.  */
#ifndef tasks_settings
#define tasks_settings

#define DEFAULT_TIMEOUT 80000 //100ms on RAT

#define ADVERTISE_PERIOD 50000 //400ms

#define COMMAND_RX_PERIOD 200000 //2s
#define COMMAND_RX_TIMEOUT 400000 // 500ms on RAT

#include "task_struct.h"

extern struct Task toggleGreenTask;
extern struct Task toggleRedTask;
extern struct Task advertiseNodeTask;
extern struct Task commandRxTask;
extern struct Task pingBaseStationTask;
extern struct Task writeUARTQueueTask;
extern struct Task initUARTTask;

extern void addTask(struct Task *task, uint32_t timeUntilFirst);

#endif
