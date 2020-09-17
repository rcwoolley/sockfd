SOURCES=scm_functions.c recvfd.c sendfd.c

OBJECTS=$(SOURCES:.c=.o)

sendfd_OBJ=sendfd.o scm_functions.o
recvfd_OBJ=recvfd.o scm_functions.o

PROGRAMS=sendfd recvfd

.PHONY: all clean rebuild

all: $(PROGRAMS)

scm_functions.h:
	curl -O "https://man7.org/tlpi/code/online/dist/sockets/scm_functions.h"

scm_functions.c: scm_functions.h
	curl -O "https://man7.org/tlpi/code/online/dist/sockets/scm_functions.c"

clean:
	$(RM) $(PROGRAMS)
	$(RM) $(OBJECTS)

rebuild: clean all

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

sendfd: $(sendfd_OBJ)
	$(CC) $(LDFLAGS) $? -o $@

recvfd: $(recvfd_OBJ)
	$(CC) $(LDFLAGS) $? -o $@
