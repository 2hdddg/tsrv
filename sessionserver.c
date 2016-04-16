#include <errno.h>
#include <string.h>
#include <sys/select.h>

#include <stdio.h>
#include <stdbool.h>

#include "log.h"
#include "control.h"
#include "protocol.h"
#include "sessionserver.h"

void sessionserver_start(int clientfd, int controlpipe)
{
    control_init(controlpipe);
    protocol_init(clientfd);

    fd_set reads;
    fd_set writes;
    int highest;
    int selected;

    while (1){
        FD_ZERO(&reads);
        FD_ZERO(&writes);
        highest = 0;

        control_prepare(&reads, &highest);
        protocol_prepare(&reads, &writes, &highest);

        selected = select(
            highest + 1, &reads, NULL, NULL, NULL);
        if (selected > 0){
            control_consume(&reads);
            protocol_consume(&reads, &writes);
        }
        else if (selected < 0){
            int error = errno;
            if (error != EINTR){
                log_print(LOG_ERR, "Select failed: %s", strerror(errno));
            }
        }

        /* Check states */
        enum controlstate controlstate = control_get_state();
        if (controlstate & controlstate_exit){
            break;
        }

        enum protocolstate protocolstate = protocol_get_state();
        if (protocolstate & protocolstate_disconnected){
            break;
        }
    }
}

