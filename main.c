#include <signal.h>

#include "log.h"
#include "server.h"

static void shutdown(int signo)
{
	printf("\nReceived SIGINT, will shutdown!\n");
	server_stop();
}

int main(int argc, char *argv[])
{
	int port = 6666;

	log_init();
	log_print(LOG_INFO, "Initializing");	
	signal(SIGINT, shutdown);

	return server_start(port);
}
