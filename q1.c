#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
// #include <sys/types.h>

// Bash command to execute: gcc q1.c -o q1 && ./q1

// OUTPUT
// n = 1
// n = 3

int n = 0;

int main(int argc, char *argv[]) {
	pid_t child;
	int status, fd[2];
	char *myargv[] = {"q1", "stop", NULL};
	char buf[10];
	n++;
	if(argc > 1) {
		// command line arguments were passed
		read(0, &n, sizeof(n));
		printf("n = %d\n", n);
		exit(3);
	}
	pipe(fd);
	child = fork();
	if(child != 0){
		// parent
		write(fd[1], &n, sizeof(n));
		waitpid(child, &status, 0);
		n = WEXITSTATUS(status);
	} else {
		// child
		n++;
		dup2(fd[0], 0);

		// The exec() family of functions replaces the current process image
        // with a new process image.  The functions described in this manual
        // page are front-ends for execve(2).
		// The execve function is most commonly used to overlay a process image that has been created by a call to the fork function.
		execve("q1", myargv, NULL);
	}
	printf("n = %d\n", n);
	return 0;
}
