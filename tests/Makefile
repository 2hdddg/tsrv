CC=gcc
CFLAGS=-c -I. -I.. -std=gnu99 -Wall
LD=gcc
LDFLAGS=
TESTS=test_telnet_parser test_receivebuffer
SOURCES=$(wildcard ../*.c)
TESTOBJECTS=$(TESTS:%=%.o)
OBJECTS=$(notdir $(SOURCES:%.c=%.o))

.PHONY:
run: $(TESTS)
	@for TEST in $(TESTS) ; do \
		./$$TEST ; \
	done

test_telnet_parser: test_telnet_parser.o telnet_parser.o sendbuffer.o
	@echo Generating $@
	@$(LD) -o $@ $^ $(LDFLAGS)

test_receivebuffer: test_receivebuffer.o receivebuffer.o
	@echo Generating $@
	@$(LD) -o $@ $^ $(LDFLAGS)

$(TESTOBJECTS): %.o: %.c
	@echo Compiling test $<
	@$(CC) $(CFLAGS) $< -o $@

$(OBJECTS): %.o: ../%.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	@echo Cleaning
	@-rm *.o -f
	@-rm $(TESTS) -f
