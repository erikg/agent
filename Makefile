# $Id: Makefile,v 1.9 2008/01/04 20:18:52 erik Exp $

CFLAGS=-D__USE_BSD -W -Wall -ansi -pedantic
CFLAGS_R=$(CFLAGS) -O2
CFLAGS_P=$(CFLAGS) -O0 -pg -ggdb
CFLAGS_D=$(CFLAGS) -O0 -ggdb -DDEBUG
CFLAGS_O=$(CFLAGS) -O3 -march=pentium -mcpu=pentium -ffast-math

SRC=agent.c bots.regex

STATE=agent.state

all: agent debot.sh

install: agent debot.sh
	install -m 555 debot.sh $(HOME)/bin/debot.sh
	strip agent && cat agent > /usr/lib/cgi-bin/agent
	> /usr/lib/cgi-bin/${STATE}
	> /tmp/agent.unknown

default: agent
prof: agent-prof
debug: agent-debug

debian:
	(cd /usr/lib/cgi-bin && ./agent)

x: agent
	@dd if=/dev/zero of=agent.state count=0
	@./agent

agent: agent.o
	$(CC) $(CFLAGS_R)  -o agent agent.o
agent.o: $(SRC) bots.regex
	$(CC) $(CFLAGS_R) -c $<


agent-prof: agent-prof.o
	$(CC) $(CFLAGS_P) -o agent-prof agent-prof.o
agent-prof.o: $(SRC)
	$(CC) $(CFLAGS_P) -o agent-prof.o -c $(SRC)

	
agent-debug: agent-debug.o
	$(CC) $(CFLAGS_D) -o agent-debug agent-debug.o
agent-debug.o: $(SRC)
	$(CC) $(CFLAGS_D) -o agent-debug.o -c $(SRC)

debot.sh: bots.regex
	echo "#!/bin/sh" > $@
	echo "grep -vi `sed 's/|/\\\\|/g' bots.regex`" >> $@

bots.regex: bots
	cat bots | xargs | sed -e 's/ /|/g' -e 's/.*/"&"/' > $@

clean:
	rm -f agent agent.o agent-prof.o agent-prof agent-debug.o agent-debug *.core bots.regex debot.sh
