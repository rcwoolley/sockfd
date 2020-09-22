# Copyright (c) 2020, Wind River Systems, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Written by Rob Woolley <rob.woolley@windriver.com>

SOURCES=passfd.c recvfd.c sendfd.c

OBJECTS=$(SOURCES:.c=.o)

sendfd_OBJ=sendfd.o passfd.o
recvfd_OBJ=recvfd.o passfd.o

PROGRAMS=sendfd recvfd

.PHONY: all clean rebuild

all: $(PROGRAMS)

debug: CFLAGS += -DDEBUG -g
debug: all

unix_passfd_test.c:
	curl "https://svnweb.freebsd.org/base/stable/12/tests/sys/kern/unix_passfd_test.c?view=co" -o $@

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
