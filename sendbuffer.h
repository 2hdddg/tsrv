#ifndef _SENDBUFFER_
#define _SENDBUFFER_

#include <sys/select.h>

typedef void* SHandle;

enum sent {
    sent_nothing,
    sent_all,
    sent_some,
    sent_error
};

SHandle sendbuffer_alloc(int fd, int max_size);
void sendbuffer_free(SHandle h);

void sendbuffer_prepare(SHandle h, fd_set *set, int *highest);
enum sent sendbuffer_flush(SHandle h, fd_set *set);
int sendbuffer_send(SHandle h, char *buffer, int size);
int sendbuffer_size(SHandle h);
void sendbuffer_print(SHandle h);

#endif /*_SENDBUFFER_*/
