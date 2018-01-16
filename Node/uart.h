#include <xdc/std.h>

#ifndef uart
#define uart

void initUART();
void writeUARTQueue();
void log(char *message);
void logWithTick(char *message);

#endif
