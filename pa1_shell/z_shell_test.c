#include <stdio.h>
#include <malloc.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <wait.h>
#include <stdlib.h>

// void childFunction(int fd) {
//     FILE *ff = fdopen(fd, "r");
//     if (ff == NULL) {
//         fprintf(stderr, "\nchild unable to open FD\n");
//     }
//     int ii;
//     while (!feof(ff)) {
//         fscanf(ff, "%d", &ii);
//         fprintf(stdout, "\nchild = %d\n", ii);
//         fflush(stdout);
//     }
// }

// void parentFunction(int fd) {
//     dup2(fd, STDOUT_FILENO);
//     for (int i = 0; i < 20; ++i) {
//         fprintf(stdout, "%d", i);
//         fflush(stdout);
//         sleep(1);
//     }
//     // dup2(fd, stdout);
//     // char buff[1024];
//     // while(read(fd, buff, 1024)) {
//     //     printf("\n%s", buff);
//     // }
// }

/*

(base) ➜  pa1_shell gcc shell_test.c && ./a.out
asdf = -1%                                                                                                      (base) ➜  pa1_shell gcc shell_test.c && ./a.out
(base) ➜  pa1_shell

*/

int f(int x) { printf("%d\n", x); }

int s(int x) {
    sleep(x);
    return true;
}

#include<errno.h>

extern int errno;

int main() {
    int a1;
    char *ccc[] = {"grep", "py", NULL};
    char *ccc1[] = {"grep", "-asdfqwr3", NULL};
    char *ccc2[] = {"grep", NULL};
    errno = 0;
    int pid, exitCode;
    switch (pid = fork()) {
        case 0:
            errno=0;
            // a1 = execvp("grep", ccc);  // success && errno = 1, Operation not permitted (if pattern not found)
            // a1 = execvp("grep", ccc1);  // errno = 1, Operation not permitted
            a1 = execvp("grepeee", ccc2);  // errno = 2, return, No such file or directory
            // a1 = execvp("grep", ccc2);  // errno = 2, return, No such file or directory
            printf("c a1=%d\n", a1);
            printf("c errno=%d\n", errno);
            printf("c %s\n", strerror(errno));
            // REFER: http://mazack.org/unix/errno.php, exit code value of execvp
            exit(256+errno);
        default:
            errno = 0;
            waitpid(pid, &errno, 0);
            if (WIFEXITED(errno)) {
                // printf("a1=%d\n", a1);
                printf("p errno=%d\n", errno);
                exitCode = WEXITSTATUS(errno);
                printf("p exitcode=%d\n", exitCode);
                printf("p %s\n", strerror(exitCode));
            }
    }
    return 0;
    // int parentPID = getpid();
    // printf("Before fork ---> pid, ppid = %d, %d\n", getpid(), getppid());
    // fork() || (fork() && fork());
    // printf("After fork ---> pid, ppid = %d, %d\n", getpid(), getppid());
    // if(getpid()==parentPID) sleep(1);
    // return 0;

    int *val;
    val = malloc(sizeof(int));
    *val = 12;
    printf("%p, %d\n", val, *val);

    switch (fork()) {
        case 0:
            sleep(0.5);
            *val = 100;
            printf("%p, %d\n", val, *val);
            free(val);
            break;

        default:
            sleep(2);
            printf("%p, %d\n", val, *val);
            free(val);
            break;
    }
    return 0;

    // char buffer[1000];
    // char *cmds[] = {"sleep", "2", NULL, "asdf", NULL};
    // char* res = strcpy(buffer, cmds[0]);
    // printf("res=%d\n", res);
    // printf("res=%c\n", res);
    // printf("res=%s\n", res);
    // return 0;
    // // int res = execvp(cmds[0], cmds);
    // // printf("asdf = %d", res);


    // // int fd[2];
    // // pipe(fd);

    // int *returncode;
    // int pid = fork();
    // switch (pid) {
    //     case -1:
    //         exit(1);
    //     case 0:
    //         // child
    //         // exit(12);
    //         execvp(cmds[0], cmds);
    //         exit(123);
    //         // childFunction(fd[0]);  // read
    //         break;
    //     default:
    //         // parent
    //         returncode = malloc(sizeof(int));

    //         //https://stackoverflow.com/questions/27306764/capturing-exit-status-code-of-child-process#:~:text=1%20Answer&text=You%20can%20get%20the%20exit,WIFEXITED%20and%20WEXITSTATUS%20with%20it.&text=waitpid()%20will%20block%20until,the%20supplied%20process%20ID%20exits.
    //         if ( waitpid(pid, returncode, 0) == -1 ) {
    //             perror("waitpid() failed");
    //             exit(EXIT_FAILURE);
    //         }

    //         if ( WIFEXITED(*returncode) ) {
    //             int es = WEXITSTATUS(*returncode);
    //             printf("Exit status was %d\n", es);
    //         }
    //         // waitpid(pid, returncode, WUNTRACED | WNOHANG);
    //         // printf("%d", returncode);
    //         // parentFunction(fd[1]);  // write
    //         break;
    // }
    // return 0;
}
