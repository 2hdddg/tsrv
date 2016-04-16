#ifndef _LOG_
#define _LOG_
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>

void log_init();
void log_close();

#define log_print(priority, format...) \
do { \
    printf("%d:", getpid()); \
    printf(format); \
    printf("\n"); \
    syslog(priority, format); \
} while (0)

#define log_trace(format...) \
do { \
    printf("%d:", getpid()); \
    printf(format); \
    printf("\n"); \
} while (0)

#endif // _LOG_
