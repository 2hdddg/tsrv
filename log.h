#ifndef _LOG_
#define _LOG_
#include <stdio.h>
#include <syslog.h>

void log_init();
void log_close();

#define log_print(priority, format...) \
	do { \
		printf(format); \
		printf("\n"); \
		syslog(priority, format); \
	} while (0)

#endif // _LOG_
