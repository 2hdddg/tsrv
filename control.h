#ifndef _CONTROLSTATE_
#define _CONTROLSTATE_
#include <sys/time.h>
#include "receivebuffer.h"

enum controlstate {
    controlstate_exit = 1
};

bool control_init(int fd);
void control_prepare(fd_set *set, int *highest);
bool control_consume(fd_set *set);
enum controlstate control_get_state();

#endif/*LSTATE_*/
