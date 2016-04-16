#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "log.h"
#include "sendbuffer.h"

struct sendbuffer {
    int     fd;
    char    *buffer;
    int     max;
    int     offset;
};

static struct sendbuffer* _from_h(SHandle h)
{
    return (struct sendbuffer*)h;
}

SHandle sendbuffer_alloc(int fd, int max_size)
{
    struct sendbuffer *me = NULL;
    char *buffer = NULL;

    me = malloc(sizeof(struct sendbuffer));
    if (!me) goto onerror;

    buffer = malloc(max_size);
    if (!buffer) goto onerror;

    me->fd = fd;
    me->buffer = buffer;
    me->max = max_size;
    me->offset = 0;

    return me;

onerror:
    if (me) free(me);
    if (buffer) free(buffer);

    return NULL;
}

void sendbuffer_free(SHandle h)
{
    struct sendbuffer *me = _from_h(h);

    free(me->buffer);
    free(me);
}

void sendbuffer_prepare(SHandle h, fd_set *set, int *highest)
{
    struct sendbuffer *me = _from_h(h);

    /* Nothing to send */
    if (me->offset == 0){
        return;
    }

    FD_SET(me->fd, set);
    if (me->fd > *highest){
        *highest = me->fd;
    }
}

int sendbuffer_send(SHandle h, char *data, int size)
{
    struct sendbuffer *me = _from_h(h);

    int left = me->max - me->offset;
    if (left > 0){
        int num = size <= left ?
            size : left;
        char *dest = me->buffer + me->offset;
        memcpy(dest, data, num);
        me->offset += num;

        return num;
    }

    return 0;
}

enum sent sendbuffer_flush(SHandle h, fd_set *set)
{
    struct sendbuffer *me = _from_h(h);

    if (!FD_ISSET(me->fd, set)){
        return sent_nothing;
    }

    if (me->offset <= 0){
        return sent_nothing;
    }

    int written = send(me->fd,
        me->buffer, me->offset,
        MSG_DONTWAIT);

    if (written < 0){
        log_print(LOG_ERR, "Unable to write: %s", strerror(errno));
        return sent_error;
    }

    log_trace("Sendbuffer flushed: %d of %d", written, me->offset);
    int left = me->offset - written;
    if (left > 0){
        /* Always sending from start of
         * buffer means we have to get
         * rid of the stuff we wrote.
        */
        me->offset = left;
        char *bufS = me->buffer;
        char *bufO = me->buffer + written;
        while (left--){
            *bufS++ = *bufO++;
        }
        return sent_some;
    }
    else{
        me->offset = 0;
        return sent_all;
    }
}

int sendbuffer_size(SHandle h)
{
    struct sendbuffer *me = _from_h(h);

    return me->max - me->offset;
}

void sendbuffer_print(SHandle h)
{
    struct sendbuffer *me = _from_h(h);

    log_trace("Sendbuffer: %p (max: %d, off: %d)",
        h, me->max, me->offset);
}
