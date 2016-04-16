#ifndef _RECEIVEBUFFER_
#define _RECEIVEBUFFER_
#include <stdbool.h>
#include <sys/select.h>

typedef void* RHandle;

enum received {
    received_nothing,
    received_data,
    received_endoffile,
    received_error
};

RHandle receivebuffer_alloc(int fd, int max_size);
void receivebuffer_free(RHandle h);

void receivebuffer_prepare(RHandle h, fd_set *set, int *highest);
enum received receivebuffer_receive(RHandle h, fd_set *set);
int receivebuffer_get(RHandle h, char **buffer);
void receivebuffer_pop(RHandle h, int num);
void receivebuffer_print(RHandle h);

#endif /*_RECEIVEBUFFER_*/
