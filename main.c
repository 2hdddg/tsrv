#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include "log.h"
#include "server.h"
#include "session.h"
#include "signalling.h"

int main(int argc, char *argv[])
{
    int port = 6666;
    bool success = true;

    log_init();

    success = session_init();
    if (!success){
        log_print(LOG_CRIT, "Failed to initialize session module");
        return 1;
    }

    success = server_init();
    if (!success){
        log_print(LOG_CRIT, "Failed to initialize server module");
        return 1;
    }

    success = signalling_init();
    if (!success){
        log_print(LOG_CRIT, "Failed to initialize signalling");
    }

    /* Note that start will hang until shutdown when successful
     * or return false immediately on failure. If something 
     * critically happens during runtime it will return false.
    */
    success = server_start(port);
    if (!success){
        return 1;
    }
    return 0;
}
