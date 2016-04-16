#ifndef _PROTOCOLSTATE_
#define _PROTOCOLSTATE_

enum protocolstate {
    protocolstate_disconnected = 1,
};

bool protocol_init(int fd);
void protocol_prepare(fd_set *reads, fd_set *writes, int *highest);
bool protocol_consume(fd_set *reads, fd_set *writes);
enum protocolstate protocol_get_state();



#endif/*_PROTOCOLSTATE_*/
