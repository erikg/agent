STATE=agent.state

all: agent

install: agent
	> /usr/lib/cgi-bin/${STATE}
	strip agent && cat agent > /usr/lib/cgi-bin/agent

default: agent
all: agent agent-prof agent-debug
prof: agent-prof
debug: agent-debug

x: agent
	@dd if=/dev/zero of=agent.state count=0
	@./agent

agent: agent.o
	gcc -O2 -o agent agent.o
agent.o: agent.c
	gcc -D__USE_BSD -O2 -c agent.c -Wall -W


agent-prof: agent-prof.o
	gcc -O3 -pg -march=pentium -ffast-math -o agent-prof agent-prof.o
agent-prof.o: agent.c
	gcc -O3 -march=pentium -pg -o agent-prof.o -c agent.c

	
agent-debug: agent-debug.o
	gcc -O0 -ggdb -o agent-debug agent-debug.o
agent-debug.o: agent.c
	gcc -O0 -DDEBUG -ggdb -o agent-debug.o -c agent.c

clean:
	rm -f agent agent.o agent-prof.o agent-prof agent-debug.o agent-debug *.core
