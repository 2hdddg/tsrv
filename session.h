#ifndef _SESSION_
#define _SESSION_
#include <stdbool.h>

bool session_init();

/* Starts a new session for the specified
   connection. The session will be handled
   in a separate process.
*/
bool session_start(int clientfd);

/* Should be called by main process when
 * a child process has ended.
 * It is assumed that main process sets up
 * a signal handler listening to SIGCHLD 
 * that this function is called from. 
*/
void session_ended(pid_t pid, int status);

void session_end_all();

#endif
