CC=gcc
CFLAGS=-c -I.
LD=gcc
LDFLAGS=
SOURCES=log.c main.c server.c
OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) tsrv 

clean:
	rm *.o tsrv 

tsrv: $(OBJECTS) 
	$(LD) -o tsrv  $(OBJECTS) $(LDFLAGS) 

.c.o:
	$(CC) $(CFLAGS) $< -o $@

