#include <string.h>

#include "telnet.h"
#include "telnet_options.h"


struct option {
    bool    is_known;
    bool    server_on;
    bool    client_on;
};

static struct option _options[256];

void telnet_option_init()
{
    memset(_options, 0, sizeof(_options));

    _options[telnet_option_transmit_binary].is_known = true;
    _options[telnet_option_echo].is_known = true;
    _options[telnet_option_suppress_go_ahead].is_known = true;
    _options[telnet_option_status].is_known = true;
    _options[telnet_option_timing_mark].is_known = true;
    _options[telnet_option_extended_options_list].is_known = true;
}
