CC=gcc
CFLAGS=-c -I. -I.. -std=gnu99 -Wall
LD=gcc
LDFLAGS=
TESTS=test_telnet_parser test_receivebuffer
SOURCES=$(wildcard ../*.c)
TESTOBJECTS=$(TESTS:%=%.o)
OBJECTS=$(notdir $(SOURCES:%.c=%.o))
TESTDEPS=$(TESTOBJECTS:%.o=%.d)
DEPS=$(OBJECTS:%.o=%.d)

.PHONY:
run: $(TESTS)
	@echo $(DEPS)
	@for TEST in $(TESTS) ; do \
		./$$TEST ; \
	done

ifneq ($(MAKECMDGOALS),clean)
-include $(TESTDEPS)
include $(DEPS)
endif

test_telnet_parser: test_telnet_parser.o telnet_parser.o sendbuffer.o
	@echo Generating $@
	@$(LD) -o $@ $^ $(LDFLAGS)

test_receivebuffer: test_receivebuffer.o receivebuffer.o
	@echo Generating $@
	@$(LD) -o $@ $^ $(LDFLAGS)

$(TESTOBJECTS): %.o: %.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) $< -o $@

$(OBJECTS): %.o: ../%.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) $< -o $@

$(TESTDEPS): %.d: %.c
	@echo Analyzing dependencies for $<
	@$(CC) -I.. -MM $(CPPFLAGS) -MT '$@ $(basename $<).o' $< > $@;

$(DEPS): %.d: ../%.c
	@echo Analyzing dependencies for $<
	@$(CC) -I.. -MM $(CPPFLAGS) -MT '$@ $(notdir $(basename $<).o)' $< > $@;

.PHONY: clean
clean:
	@echo Cleaning
	@-rm *.o -f
	@-rm *.d -f
	@-rm $(TESTS) -f
