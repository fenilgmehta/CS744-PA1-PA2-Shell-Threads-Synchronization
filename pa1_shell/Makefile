# REFER: https://youtu.be/aw9wHbFTnAQ

# declare the variable
CC=gcc
CFLAGS_STABLE=
CFLAGS_DEV=-O3 -Wall -DPRINT_DEBUG_STATEMENTS

# -----------------------------------------------------

# by default, the first target will be executed
all: shell_fm

shell_fm: shell_fm.c
	$(CC) $(CFLAGS_STABLE) shell_fm.c -o shell_fm

# -----------------------------------------------------

all_dev: shell_fm_dev

shell_fm_dev: shell_fm.c
	$(CC) $(CFLAGS_DEV) shell_fm.c -o shell_fm_dev

# -----------------------------------------------------

# clean the build output
clean:
	rm -f shell_fm shell_fm_dev
