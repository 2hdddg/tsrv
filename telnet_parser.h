#ifndef _TELNET_COMMANDS_
#define _TELNET_COMMANDS_

#include "sendbuffer.h"

void telnet_parser_init();

struct telnet_commands {
    /* Buffer containing zero or more commands. */
    unsigned char   *commands;
    /* Number of bytes in above buffer*/
    int             size;
    /* Number of different commands in buffer */
    int             count;
};

/* Splits 'text' into a stream of text.
 * and another stream of commands.
 * Commands is telnet protocol commands.
 * and text is to be consumed by terminal.
 * Note that content of text buffer will be
 * changed by this function!
 *
 * text - Buffer of chars, typically received from client.
 * size - Size of above buffer.
 * parsed - Output, contains parsed commands, the buffer
 *          is owned by the parser and will contain valid
 *          data until next call to parse.
 *
 * Returns number of bytes in text buffer or negative
 * on error
*/
int telnet_parser_parse(
    char* text, int size,
    struct telnet_commands *commands);

/*
 * Returns:
 * > 0 when command has been sent
 * 0 when nothing has been sent
 * < 0 when error encountered
*/
int telnet_will_transmit_binary(SHandle h);

#endif /*_TELNET_COMMANDS_*/
