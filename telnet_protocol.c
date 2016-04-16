#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include "log.h"
#include "receivebuffer.h"
#include "sendbuffer.h"
#include "protocol.h"
#include "telnet_parser.h"
#include "telnet_options.h"

#define MAX_CLIENT_READ     (255)
#define MAX_CLIENT_WRITE    (255)

static RHandle _rb = NULL;
static SHandle _sb = NULL;
static enum protocolstate _protocolstate = 0;

bool protocol_init(int fd)
{
    /* Need to send stuff immediately, uses own send buffer to
     * avoid too many small writes.
    */
    int one = 1;
    if (setsockopt(fd, SOL_TCP, TCP_NODELAY, &one, sizeof(one)) == -1){
        log_print(LOG_CRIT, "Failed to set TCP_NODELAY: %s", strerror(errno));
        goto onerror;
    }

    _rb = receivebuffer_alloc(fd, MAX_CLIENT_READ);
    if (!_rb){
        goto onerror;
    }

    _sb = sendbuffer_alloc(fd, MAX_CLIENT_WRITE);
    if (!_sb){
        goto onerror;
    }

    telnet_option_init();
    telnet_parser_init();

    telnet_will_transmit_binary(_sb);

    return true;

onerror:
    if (_rb){
        receivebuffer_free(_rb);
        _rb = NULL;
    }
    if (_sb){
        sendbuffer_free(_sb);
        _sb = NULL;
    }
    return false;
}

void protocol_prepare(fd_set *reads, fd_set *writes, int *highest)
{
    /* Always reading */
    receivebuffer_prepare(_rb, reads, highest);
    /* Will request write if anything in buffer */
    sendbuffer_prepare(_sb, writes, highest);
}

static void _read_client()
{
    int size;
    char *buffer;
    struct telnet_commands commands;

    size = receivebuffer_get(_rb, &buffer);
    if (size < 0){
        log_print(LOG_ERR, "Error during read");
        return;
    }

    int text_size = telnet_parser_parse(buffer, size, &commands);
    if (text_size >= 0){
        log_trace("Received %d bytes of text", text_size);
    }
    else{
    }

    receivebuffer_pop(_rb, size);
}

bool protocol_consume(fd_set *reads, fd_set *writes)
{
    switch (receivebuffer_receive(_rb, reads)){
        case received_data:
            _read_client();
            break;
        case received_endoffile:
            log_print(LOG_INFO, "Client disconnected");
            _protocolstate |= protocolstate_disconnected;
            break;
        case received_error:
            log_print(LOG_ERR, "Unable to receive");
            return false;
        case received_nothing:
        default:
            break;
    }

    switch (sendbuffer_flush(_sb, writes)){
        case sent_nothing:
        case sent_some:
        case sent_all:
            //_write_client();
            break;
        case sent_error:
            log_print(LOG_ERR, "Unable to send");
            return false;
    }

    return true;
}

enum protocolstate protocol_get_state()
{
    return _protocolstate;
}
