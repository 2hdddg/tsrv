#include <syslog.h>
#include "log.h"

void log_init()
{
	openlog("tsrv", LOG_PID|LOG_CONS, LOG_USER);
}

void log_close()
{
	closelog();
}

