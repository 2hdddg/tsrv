#ifndef _TELNET_
#define _TELNET_

/* Indicates that a command comes next */
#define TELNET_IAC 255

enum telnet_command {
    /* Beginning of subnegotiations */
    telnet_command_sb = -1,
    /* End of subnegotiation parameters*/
    telnet_command_se = 240,
    telnet_command_nop = 241,
    telnet_command_data_mark = 242,
    telnet_command_will = 251,
    telnet_command_wont = 252,
    telnet_command_do = 253,
    telnet_command_dont = 254,
};

enum telnet_option {
    telnet_option_transmit_binary = 0,
    telnet_option_echo = 1,
    telnet_option_suppress_go_ahead = 3,
    telnet_option_status = 5,
    telnet_option_timing_mark = 6,
    telnet_option_extended_options_list = 255,
};

#endif /*_TELNET_*/
