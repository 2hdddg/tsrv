#include <stdio.h>
#include <stdbool.h>

#include "log.h"
#include "telnet.h"
#include "telnet_parser.h"

#define MAX_COMMAND_SIZE 25

unsigned char _buffer[MAX_COMMAND_SIZE];
int _length;
int _last_command_offset;
int _num_commands;

int (*_state)(unsigned char);

static int _state_check_for_iac(unsigned char c);
static int _state_await_command(unsigned char c);
static int _state_await_option(unsigned char c);

static int _append(unsigned char c)
{
    if (_length < (MAX_COMMAND_SIZE - 1)){
        _length++;
        _buffer[_length] = c;
        return 1;
    }
    else{
        log_print(LOG_ERR, "Parse buffer full");
        return -1;
    }
}

static int _append_and_end(unsigned char c)
{
    int appended = _append(c);
    if (appended < 0){
        return appended;
    }
    _num_commands++;
    _last_command_offset = _length;

    _state = &_state_check_for_iac;

    return appended;
}

static int _state_check_for_iac(unsigned char c)
{
    if (c == TELNET_IAC){
        _state = &_state_await_command;
        return _append(c);
    }
    return 0;
}

static int _state_await_command(unsigned char c)
{
    if (c == TELNET_IAC){
        _state = &_state_check_for_iac;
        _length--;
        return 0;
    }

    switch (c){
    case telnet_command_will:
    case telnet_command_wont:
    case telnet_command_do:
    case telnet_command_dont:
        _state = &_state_await_option;
        return _append(c);
    //case telnet_command_sb:
    case telnet_command_se:
    case telnet_command_nop:
    case telnet_command_data_mark:
    default:
        log_print(LOG_ERR, "Unhandled command: %d", c);
        return -1;
    }
}

static int _state_await_option(unsigned char c)
{
    switch (c){
    case telnet_option_transmit_binary:
    case telnet_option_echo:
    case telnet_option_suppress_go_ahead:
    case telnet_option_status:
    case telnet_option_timing_mark:
        return _append_and_end(c);
    case telnet_option_extended_options_list:
    default:
        log_print(LOG_ERR, "Unhandled option: %d", c);
        return -1;
    }
}

static void _flush()
{
    if (_num_commands > 0){
        /* We have no half command in buffer, easy
         * case, no move needed
        */
        if (_last_command_offset == _length){
            _length = -1;
            _last_command_offset = _length;
            _num_commands = 0;
        }
        else{
            log_print(LOG_ERR, "Not impl!");
        }
    }
}

void telnet_parser_init()
{
    _length = -1;
    _num_commands = 0;
    _last_command_offset = _length;
    _state = &_state_check_for_iac;
}

int telnet_parser_parse(
    char* unparsed, int size, struct telnet_commands *commands)
{
    int text_count = 0;
    int command_count = 0;
    char *current = unparsed;

    _flush();

    while(size--){
        char c = *current;
        int parsed = (*_state)(c);

        switch (parsed){
            case 0:
                text_count++;
                if (command_count){
                    /* Move everything - steps */
                    char *before = current - command_count;
                    char *after = current;

                    while (--command_count){
                        *before++ = *after++;
                    }
                }
                break;

            case 1:
                command_count++;
                break;

            case -1:
                return -1;
        }
        current++;
    }

    commands->commands = _buffer;
    commands->size = _num_commands > 0 ?
         _length + 1 : 0;
    commands->count = _num_commands;

    /* Dont bother with commands at the end
     * the text_count wont include them 
     */

    return text_count;
}

int _try_send(SHandle h, char *buffer, int size)
{
    int available = sendbuffer_size(h);

    /* Commands need to be written in whole */
    if (size <= available){
        return sendbuffer_send(h, buffer, size);
    }

    return 0;
}

int telnet_will_transmit_binary(SHandle h)
{
    char command[] = { TELNET_IAC, telnet_command_will, telnet_option_transmit_binary };
    return _try_send(h, command, sizeof(command));
}
