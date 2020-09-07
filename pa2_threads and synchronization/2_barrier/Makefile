# Default to debugging flags. Set to -O3 if you want optimisations
# (either change it here, or type 'make CFLAGS=-O3' in the shell).
CFLAGS?=-g -Wall -Wno-unused-value

all: barrier_exec

barrier_exec: barrier.c barrier-testcase1.c barrier-testcase2.c barrier-testcase3.c barrier-testcase4.c barrier-testcase5.c barrier.h
	$(CC) $(CFLAGS) -o barrier1 barrier-testcase1.c barrier.c -lpthread
	$(CC) $(CFLAGS) -o barrier2 barrier-testcase2.c barrier.c -lpthread
	$(CC) $(CFLAGS) -o barrier3 barrier-testcase3.c barrier.c -lpthread
	$(CC) $(CFLAGS) -o barrier4 barrier-testcase4.c barrier.c -lpthread
	$(CC) $(CFLAGS) -o barrier5 barrier-testcase5.c barrier.c -lpthread 

clean:
	$(RM) barrier1
	$(RM) barrier2
	$(RM) barrier3
	$(RM) barrier4
	$(RM) barrier5
	$(RM) -r *.dSYM

run: barrier_exec
	./barrier1
	./barrier2
	./barrier3
	./barrier4
	./barrier5
