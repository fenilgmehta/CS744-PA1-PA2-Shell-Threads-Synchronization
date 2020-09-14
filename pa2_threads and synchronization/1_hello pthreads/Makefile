# Default to debugging flags. Set to -O3 if you want optimisations
# (either change it here, or type 'make CFLAGS=-O3' in the shell).
CFLAGS?=-g -Wall -Wno-unused-value

# REFER: https://stackoverflow.com/questions/2826029/passing-additional-variables-from-command-line-to-make
THREADS_N?=10

all: nlocks threads

nlocks: nlocks.c
	$(CC) $(CFLAGS) -o nlocks nlocks.c -lpthread

threads: threads.c
	$(CC) $(CFLAGS) -o threads threads.c -lpthread

clean:
	$(RM) nlocks
	$(RM) threads
	$(RM) -r *.dSYM

run: all
	./nlocks
	./threads

run_nlocks: nlocks
	./nlocks

run_threads: threads
	./threads $(THREADS_N)

