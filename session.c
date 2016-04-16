/* Responsible for keeping track of and creating new sessions.
 * All exported functions are to be called from main process.
*/

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "log.h"
#include "session.h"
#include "signalling.h"
#include "sessionserver.h"

#define MAX_NUM_SESSIONS 10

struct tsrv_session {
    pid_t   pid;
    bool    allocated;
    int     controlpipe[2];
};


struct tsrv_session _sessions[MAX_NUM_SESSIONS];

static struct tsrv_session* _session_alloc(int clientfd)
{
    struct tsrv_session *session = NULL;

    for (int i=0; i < MAX_NUM_SESSIONS; i++){
        session = &_sessions[i];
        if (!session->allocated){
            int piped = pipe(session->controlpipe);
            if (piped == -1){
                log_print(LOG_ERR,
                    "Failed to allocate session pipe: %s", strerror(errno));
            }
            session->allocated = true;
            return session;
        }
    }
    log_print(LOG_ERR, "Failed to find slot for session");
    return NULL;
}

static struct tsrv_session* _session_find(pid_t pid)
{
    struct tsrv_session* session;

    for (int i=0; i < MAX_NUM_SESSIONS; i++){
        session = &_sessions[i];
        if (session->pid == pid){
            return session;
        }
    }
    return NULL;
}

static bool _session_free(struct tsrv_session* session)
{
    if (!session){
        log_print(LOG_ERR, "Trying to free NULL session");
        return false;
    }

    if (!session->allocated){
        log_print(LOG_ERR, "Trying to free already freed session");
        return false;
    }

    close(session->controlpipe[0]);
    close(session->controlpipe[1]);
    session->allocated = false;
    session->pid = 0;

    return true;
}

bool session_init()
{
    size_t size = sizeof(struct tsrv_session) * MAX_NUM_SESSIONS;
    memset(_sessions, 0, size);
    return true;
}

/* Starts a new child process that handles
 * the session.
*/
bool session_start(int clientfd)
{
    log_print(LOG_INFO, "Starting new session");
    struct tsrv_session* session = _session_alloc(clientfd);
    if (!session){
        log_print(LOG_ERR, "Failed to allocate slot for new session");
        return false;
    }

    pid_t pid = fork();
    if (pid > 0){
        /* Still in main process
         * Keep track of the child process to
         * enable shutdown and perhaps status
         * checks
        */
        session->pid = pid;
        return true;
    }
    else if (pid == 0){
        /* In forked process
        */
        log_init();

        sessionserver_start(clientfd, session->controlpipe[0]);
        /* Child process should exit here to stop backtracing
        */
        log_trace("Closing connection");
        shutdown(clientfd, SHUT_RDWR);
        close(clientfd);
        _exit(0);
    }
    else{
        log_print(LOG_ERR, "Failed to fork new session: %s", strerror(errno));
        return false;
    }
}

void session_ended(pid_t pid, int status)
{
    struct tsrv_session *session = _session_find(pid);

    if (session){
        _session_free(session);

        if (WIFEXITED(status)){
            int exitstatus = WEXITSTATUS(status);
            if (exitstatus == 0){
                log_trace("Session process %d ended successfully", pid);
            }
            else{
                log_print(LOG_ERR,
                    "Session process %d ended with an error: %d", pid, exitstatus);
            }
        }
        else{
            log_print(LOG_ERR,
                "Session process %d ended in an unexpected way: %d", pid, status);
        }
    }
    else{
        log_print(LOG_ERR,
            "Unable to find session for ended child process %d", pid);
    }
}

void session_end_all()
{
    for (int i=0; i<MAX_NUM_SESSIONS; i++){
        struct tsrv_session *session = &_sessions[i];

        if (session->allocated){
            int fd = session->controlpipe[1];
            write(fd, "end", 4);
        }
    }

    int status;
    pid_t pid = 0;

    while ((pid = waitpid(-1, &status, 0)) > 0){
        if (errno == ECHILD){
            break;
        }
    }
}
