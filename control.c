#include <assert.h>
#include <string.h>

#include "log.h"
#include "receivebuffer.h"
#include "control.h"

#define MAX_CONTROL_SIZE (10)

static RHandle _rb;
static enum controlstate _state = 0;

bool _evaluate_command(char* command)
{
    if (strcmp(command, "end") == 0){
        log_trace("Received end command from controller");
        _state = controlstate_exit;
        return true;
    }
    else{
        log_print(LOG_ERR, "Unknown command: %s", command);
    }

    return false;
}

bool control_init(int fd)
{
    _rb = receivebuffer_alloc(fd, MAX_CONTROL_SIZE);
    if (!_rb){
        return false;
    }
    return true;
}

void control_prepare(fd_set *set, int *highest)
{
    receivebuffer_prepare(_rb, set, highest);
}

static bool _read()
{
    int size;
    char *buffer;
    char *command = NULL;

    size = receivebuffer_get(_rb, &buffer);
    if (size < 0){
        log_print(LOG_ERR, "Error during read");
        return false;
    }

    /* Checks if the buffer contains a complete.
     * command. Returns the command if it exists
     * otherwise NULL.
    */
    for (int i = 0; i < size; i++){
        if (buffer[i] == 0){
            receivebuffer_pop(_rb, i);
            command = buffer;
            break;
        }
    }

    if (command){
        return _evaluate_command(command);
    }

    return false;
}

bool control_consume(fd_set *set)
{
    enum received received;

    received = receivebuffer_receive(_rb, set);
    switch (received){
        case received_data:
            return _read();
        case received_error:
            log_print(LOG_ERR, "Unable to receive");
            return false;
        case received_endoffile:
        case received_nothing:
        default:
            return false;
    }
}

enum controlstate control_get_state()
{
    return _state;
}
