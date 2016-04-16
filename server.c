/* Listens for new connections.
 * Executes in main process.
*/
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "log.h"
#include "session.h"
#include "server.h"

int _listenfd = -1;
bool _is_shutting_down = false;

static bool _create_socket(int port)
{
    _listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenfd == -1){
        log_print(LOG_CRIT, "Failed to create socket: %s", strerror(errno));
        return false;
    }
    return true;
} 

static void _close_socket()
{
    if (_listenfd >= 0){
        close(_listenfd);
    }
}

static bool _bind_to_socket(int port)
{
    struct sockaddr_in localaddress;

    memset(&localaddress, 0, sizeof(struct sockaddr_in));
    localaddress.sin_family = AF_INET;
    localaddress.sin_addr.s_addr = INADDR_ANY;
    localaddress.sin_port = htons(port);

    /* This makes sure that we can reuse the port when
    *  starting/stopping. Not critical if it fails
    */
    int optval = 1;
    if (setsockopt(_listenfd,
        SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1){
        log_print(LOG_ERR, "Failed to set option to reuse port: %s", strerror(errno));
    }

    if (bind(_listenfd,
            (struct sockaddr*)&localaddress,
            sizeof(struct sockaddr_in)) == -1){
        log_print(LOG_CRIT, "Failed to bind socket: %s", strerror(errno));
        return false;
    }
    return true;
}

static bool _listen_on_socket()
{
    if (listen(_listenfd, 1) == -1){ 
        log_print(LOG_CRIT, "Failed to listen on socket: %s", strerror(errno));
        return false;
    }
    return true;
}

static bool _accept_on_socket(int *acceptedfd_out)
{
    struct sockaddr_in remoteaddress;
    socklen_t remoteaddress_len = sizeof(struct sockaddr_in);

    do {
        int acceptedfd = accept(_listenfd,
            (struct sockaddr*)&remoteaddress,
            &remoteaddress_len);
        if (acceptedfd == -1){
            int error = errno;

            switch (error){
            case EINTR:
                /* Interrupted by some other signal, like a child
                 * process that exited, retry !
                */
                log_trace("Got interrupted");
                break;
            default:
                if (!_is_shutting_down){
                    log_print(LOG_CRIT, "Failed to accept: %s", strerror(error));
                }
                return false;
            }
        }
        else{
            char addr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &remoteaddress.sin_addr, addr, INET_ADDRSTRLEN);
            log_print(LOG_INFO, "Got a connection from %s", addr);

            *acceptedfd_out = acceptedfd;
            return true;
        }
    } while (1);
}

bool server_init()
{
    return true;
}

bool server_start(int port)
{
    bool success =
        _create_socket(port) &&
        _bind_to_socket(port) &&
        _listen_on_socket();

    if (!success){
        _close_socket();
        return false;
    }

    log_trace("Accepting connections on port %d", port);
    while(success){
        int acceptedfd;
        success = _accept_on_socket(&acceptedfd);
        if (success){
            session_start(acceptedfd);
        }
        else{
            break;
        }
    }
    log_print(LOG_INFO, "Stopped accepting connections on port %d", port);
    _close_socket();
    return true;
}

void server_stop()
{
    log_print(LOG_INFO, "Initiating shutdown...");
    _is_shutting_down = true;
    _close_socket();
    session_end_all();
}
