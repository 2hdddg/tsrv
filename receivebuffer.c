#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "log.h"
#include "receivebuffer.h"

struct receivebuffer {
    int    fd;
    char   *buffer;
    int    max;
    int    received;
    int    offset;
};

static struct receivebuffer* _from_h(RHandle h)
{
    return (struct receivebuffer*)h;
}

RHandle receivebuffer_alloc(int fd, int max_size)
{
    struct receivebuffer *me = malloc(
        sizeof(struct receivebuffer));
    if (!me){
        return NULL;
    }
    char *buffer = malloc(max_size);
    if (!buffer){
        free(me);
        return NULL;
    }

    me->fd = fd;
    me->received = 0;
    me->buffer = buffer;
    me->max = max_size;
    me->offset = 0;

    return me;
}

void receivebuffer_free(RHandle h)
{
    struct receivebuffer *me = _from_h(h);
    free(me->buffer);
    me->buffer = NULL;
    free(me);
}

void receivebuffer_prepare(RHandle h, fd_set *set, int *highest)
{
     struct receivebuffer *me = _from_h(h);

     FD_SET(me->fd, set);
     if (me->fd > *highest){
         *highest = me->fd;
    }
}

static void _flush(struct receivebuffer *me)
{
    int movement = me->received - me->offset;
    if (movement > 0){
        /* Copy */
        char *bufS = me->buffer;
        char *bufO = me->buffer + me->offset;
        while (movement--){
            *bufS++ = *bufO++;
        }
        me->received -= me->offset;
        me->offset = 0;
    }
    else{
        me->received = 0;
        me->offset = 0;
    }
}

static int _receive(struct receivebuffer *me)
{
    _flush(me);

    int num_to_read = me->max - me->received;
    if (num_to_read <= 0){
        log_print(LOG_ERR, "Receive buffer is full!");
        return -1;
    }

    char* buffer = me->buffer;
    assert(me->buffer);

    buffer += me->received;
    int num_read = read(me->fd, buffer, num_to_read);
    log_trace("Receivebuffer read: %d", num_read);

    me->received += num_read;

    return num_read;
}

enum received receivebuffer_receive(RHandle h, fd_set *set)
{
    struct receivebuffer *me = _from_h(h);

    if (FD_ISSET(me->fd, set)){
        int received = _receive(me);
        if (received > 0){
            return received_data;
        }
        else if (received == 0){
            return received_endoffile;
        }
        else{
            return received_error;
        }
    }
    else{
        return received_nothing;
    }

}

int receivebuffer_get(RHandle h, char **buffer)
{
    struct receivebuffer *me = _from_h(h);
    int size = me->received - me->offset;

    if (size > 0){
        *buffer = me->buffer + me->offset;
    }
    else{
        *buffer = NULL;
    }

    return size;
}

void receivebuffer_pop(RHandle h, int num)
{
    struct receivebuffer *me = _from_h(h);

    int max = me->received - me->offset;
    num = num <= max ? num : max;
    me->offset += num;
}

void receivebuffer_print(RHandle h)
{
    struct receivebuffer *me = _from_h(h);

    log_trace("Receivebuffer: %p (max: %d, rec: %d, off: %d)",
        h, me->max, me->received, me->offset);
}
