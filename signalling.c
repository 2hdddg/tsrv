#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#include "log.h"
#include "server.h"
#include "session.h"

pid_t _main_pid;

static bool is_child()
{
    return getpid() != _main_pid;
}

static void request_shutdown(int signum, siginfo_t *info, void *ctx)
{
    if (!is_child()){
        server_stop();
    }
}

static void child_ended(int signum, siginfo_t *info, void *ctx)
{
    session_ended(info->si_pid, info->si_status);
}

static void register_signal_handlers()
{
    struct sigaction sa;

    /* Listen on SIGINT & SIGTERM to enable user to stop server
     * in a nice and clean way (dont leave any sockets open).
    */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = request_shutdown;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    sa.sa_sigaction = request_shutdown;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);

    /* Listen on SIGCHLD to detect whenever
     * a child (session) process exits so
     * that we have fresh list of sessions.
    */
    sa.sa_sigaction = child_ended;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
}

bool signalling_init()
{
    _main_pid = getpid();
    register_signal_handlers();

    return true;
}
