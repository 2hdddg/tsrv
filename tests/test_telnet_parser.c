#include <stdio.h>

#include "telnet.h"
#include "telnet_parser.h"
#include "test.h"

static void parse_text()
{
    START();
    char text[] = {'1', '2', '3'};
    struct telnet_commands commands;
    telnet_parser_init();

    int parsed =
        telnet_parser_parse(text, sizeof(text), &commands);

    ASSERT(1);

    ASSERT(parsed == 3);
    ASSERT(text[0] == '1');
    ASSERT(text[2] == '3');

    ASSERT(commands.size == 0);
    ASSERT(commands.count == 0);
}

static void parse_text_command()
{
    START();
    char text[] = {
        '1', '2',
        TELNET_IAC, telnet_command_will, telnet_option_transmit_binary};
    struct telnet_commands commands;
    telnet_parser_init();

    int parsed =
        telnet_parser_parse(text, sizeof(text), &commands);

    ASSERT(parsed == 2);
    ASSERT(text[0] == '1');
    ASSERT(text[1] == '2');

    ASSERT(commands.size == 3);
    ASSERT(commands.count == 1);
    ASSERT(commands.commands[0] == TELNET_IAC);
    ASSERT(commands.commands[1] == telnet_command_will);
    ASSERT(commands.commands[2] == telnet_option_transmit_binary);
}

static void parse_text_command_text()
{
    START();
    char text[] = {
        '1', '2',
        TELNET_IAC, telnet_command_will, telnet_option_transmit_binary,
        '3'};
    struct telnet_commands commands;
    telnet_parser_init();

    int parsed =
        telnet_parser_parse(text, sizeof(text), &commands);

    ASSERT(parsed == 3);
    ASSERT(text[0] == '1');
    ASSERT(text[1] == '2');
    ASSERT(text[2] == '3');

    ASSERT(commands.size == 3);
    ASSERT(commands.count == 1);
    ASSERT(commands.commands[0] == TELNET_IAC);
    ASSERT(commands.commands[1] == telnet_command_will);
    ASSERT(commands.commands[2] == telnet_option_transmit_binary);
}

static void parse_command()
{
    START();
    char text[] = {
        TELNET_IAC, telnet_command_will, telnet_option_transmit_binary };
    struct telnet_commands commands;
    telnet_parser_init();

    int parsed =
        telnet_parser_parse(text, sizeof(text), &commands);

    ASSERT(parsed == 0);

    ASSERT(commands.size == 3);
    ASSERT(commands.count == 1);
    ASSERT(commands.commands[0] == TELNET_IAC);
    ASSERT(commands.commands[1] == telnet_command_will);
    ASSERT(commands.commands[2] == telnet_option_transmit_binary);
}

static void parse_command_command()
{
    START();
    char text[] = {
        TELNET_IAC, telnet_command_will, telnet_option_transmit_binary,
        TELNET_IAC, telnet_command_do, telnet_option_echo };
    struct telnet_commands commands;
    telnet_parser_init();

    int parsed =
        telnet_parser_parse(text, sizeof(text), &commands);

    ASSERT(parsed == 0);

    ASSERT(commands.size == 6);
    ASSERT(commands.count == 2);
    ASSERT(commands.commands[0] == TELNET_IAC);
    ASSERT(commands.commands[1] == telnet_command_will);
    ASSERT(commands.commands[2] == telnet_option_transmit_binary);
}

static void parse_command_text_command()
{
    START();
    char text[] = {
        TELNET_IAC, telnet_command_will, telnet_option_transmit_binary,
        '1', '2',
        TELNET_IAC, telnet_command_do, telnet_option_echo };
    struct telnet_commands commands;
    telnet_parser_init();

    int parsed =
        telnet_parser_parse(text, sizeof(text), &commands);

    ASSERT(parsed == 2);

    ASSERT(commands.size == 6);
    ASSERT(commands.count == 2);
    ASSERT(commands.commands[0] == TELNET_IAC);
    ASSERT(commands.commands[1] == telnet_command_will);
    ASSERT(commands.commands[2] == telnet_option_transmit_binary);
}

static void parse_half_half_command()
{
    START();
    char text[] = {
        TELNET_IAC, telnet_command_will };
    struct telnet_commands commands;
    telnet_parser_init();


    int parsed =
        telnet_parser_parse(text, sizeof(text), &commands);

    ASSERT(parsed == 0);
    ASSERT(commands.size == 0);
    ASSERT(commands.count == 0);

    text[0] = telnet_option_transmit_binary;
    parsed +=
        telnet_parser_parse(text, 1, &commands);

    ASSERT(parsed == 0);
    ASSERT(commands.size == 3);
    ASSERT(commands.count == 1);
    ASSERT(commands.commands[0] == TELNET_IAC);
    ASSERT(commands.commands[1] == telnet_command_will);
    ASSERT(commands.commands[2] == telnet_option_transmit_binary);
}

int main()
{
    parse_text();
    parse_text_command();
    parse_text_command_text();
    parse_command();
    parse_command_command();
    parse_command_text_command();
    parse_half_half_command();

    return REPORT();
}
