#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-macro-parentheses"

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
#include <fcntl.h>
#include <sysexits.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#define printflush(a, ...) { fprintf(a, __VA_ARGS__); fflush(a); }
#define dbss(a)     printflush(stderr, "%s", a);

typedef int32_t INT;
typedef int64_t LONG;

// ---------------------------------------------------------------------------------------------------------------------

// TODO: comment this before submitting the code
// #define PRINT_DEBUG_STATEMENTS

// REFER: https://stackoverflow.com/questions/3473692/list-environment-variables-with-c-in-unix
extern char **environ;

const char *str_myShellPrompt = "myShell> ";
const char *str_SIGNAL_SIGINT = "the program is interrupted, do you want to exit [Y/N]";
const char *str_SIGNAL_SIGTERM = "Got SIGTERM-Leaving";
const char *str_ILLEGAL_COMMAND = "Illegal command or arguments";

// These are just thresh-holds to ensure faster performance and make sure realloc is not called repetitively
char inputBuffer[2048];
const int MAX_INPUT_LENGTH = 2048;
const int MAX_COMMANDS_LENGTH = 32;

// ---------------------------------------------------------------------------------------------------------------------

void myshell(bool, int *);


/* This was written because `scanf` and `read` could not handle cases where EOF
 * is reached along with the input to be read till a new line character.
 * Especially the case when the input is taken from a file with last line NOT empty.
 * Return bool: false if End-Of-File (OR CTRL+D) reached, otherwise true
 * */
bool myShellInput(char *userInput) {
    static char ch;

    char *userInputOld = userInput;
    ch = getchar();
#ifdef PRINT_DEBUG_STATEMENTS
    printflush(stderr, "DEBUG: ch=%d,%c\n", ch, ch);  // DEBUG
#endif

    while (ch != '\n') {
        if (ch == (-1)) {
            *userInput = '\0';
            return userInput != userInputOld;  // this returns false if not input read
        }
        *userInput = ch;
        ++userInput;
        ch = getchar();
#ifdef PRINT_DEBUG_STATEMENTS
        printflush(stderr, "DEBUG: ch=%d,%c\n", ch, ch);  // DEBUG
#endif
    }

    *userInput = '\0';
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------

// REFER: https://stackoverflow.com/questions/605845/do-i-cast-the-result-of-malloc?noredirect=1&lq=1

#define MyVectorTemplate(DataType, Suffix)                                      \
                                                                                \
    struct MyVector ## Suffix{                                                  \
        INT n, nMax;                                                            \
        DataType* arr;                                                          \
    };                                                                          \
                                                                                \
    /* returns: true if resize was successful */                                \
    bool mv_resize ## Suffix (struct MyVector ## Suffix *mv, INT newSize){      \
        DataType *arrNew = realloc(mv->arr, sizeof(DataType) * newSize);        \
        if(arrNew != NULL) {                                                    \
            mv->arr = arrNew;                                                   \
            mv->nMax = newSize;                                                 \
        }                                                                       \
        return arrNew != NULL;                                                  \
    }                                                                           \
                                                                                \
    /* returns: true if push_back was successful */                             \
    bool mv_push_back ## Suffix (struct MyVector ## Suffix *mv, DataType val) { \
        if(mv->n == mv->nMax) {                                                 \
            if(! (mv_resize ## Suffix (mv, mv->nMax * 2))) {                    \
                /* if resize fails */                                           \
                return false;                                                   \
            }                                                                   \
        }                                                                       \
        mv->arr[mv->n] = val;                                                   \
        mv->n += 1;                                                             \
        return true;                                                            \
    }                                                                           \
                                                                                \
    /* returns: the last element popped */                                      \
    DataType mv_pop_back ## Suffix (struct MyVector ## Suffix *mv, int popCount) {  \
        if(popCount <= 0) return mv->arr[0];                                    \
        if(mv->n <= popCount) mv->n = 0;                                        \
        else mv->n -= popCount;                                                 \
        return mv->arr[mv->n];                                                  \
    }                                                                           \
                                                                                \
    void mv_free ## Suffix (struct MyVector ## Suffix *mv) {                    \
        free(mv->arr);                                                          \
    }                                                                           \
                                                                                \
    void mv_init ## Suffix (struct MyVector ## Suffix *mv, int defSize) {       \
        mv->n = 0;                                                              \
        mv->nMax = defSize;                                                     \
        mv->arr = malloc(sizeof(DataType) * mv->nMax);                          \
    }

MyVectorTemplate(INT, _INT);
MyVectorTemplate(char, _CHAR);
MyVectorTemplate(char*, _CHARPTR);
MyVectorTemplate(pid_t, _pid);

// ---------------------------------------------------------------------------------------------------------------------

/* ASSUMPTION: Following commands are considered INVALID
 *    $ ls > asdf | less
 *    $ ls | grep ".*" < asdf.txt
 *
 * */
struct Command {
    struct MyVector_CHARPTR command;    // stores word pointers
    struct MyVector_INT commandIdx;     // stores the index of word start in parsedStr
    struct MyVector_INT inputFileIdx, outputFileIdx;    // NOTE: if value of outputFileIdx is -ve, then overwrite the file
    bool inputFromPipe, outputToPipe;
};

void command_init(struct Command *cmd) {
    mv_init_CHARPTR(&cmd->command, 128);
    mv_init_INT(&cmd->commandIdx, 128);
    mv_init_INT(&cmd->inputFileIdx, 128);
    mv_init_INT(&cmd->outputFileIdx, 128);
    cmd->inputFromPipe = cmd->outputToPipe = false;
}

struct Command *command_constructor() {
    struct Command *ptr = malloc(sizeof(struct Command));
    command_init(ptr);  // Command init is necessary to ensure the object is initialized properly
    return ptr;
}

void command_clear(struct Command *cmd) {
    cmd->command.n = cmd->commandIdx.n = cmd->inputFileIdx.n = cmd->outputFileIdx.n = 0;
    cmd->inputFromPipe = cmd->outputToPipe = false;
}

void command_free(struct Command *cmd) {
    mv_free_CHARPTR(&cmd->command);
    mv_free_INT(&cmd->commandIdx);
    mv_free_INT(&cmd->inputFileIdx);
    mv_free_INT(&cmd->outputFileIdx);
}

/* This is done after parsing the input to ensure char pointers are NOT INVALID
 * if vector size changes after change in vector size with push_back (realloc) */
void command_initCharPtrArray(struct MyVector_CHAR *parsedStrings, struct Command *cmd) {
    // int i;
    for (int i = 0; i < cmd->commandIdx.n; ++i) {
        int wordIdx = cmd->commandIdx.arr[i];
        char *wordPtr = &(parsedStrings->arr[wordIdx]);
        mv_push_back_CHARPTR(&cmd->command, wordPtr);
    }
    mv_push_back_CHARPTR(&cmd->command, NULL);
}

MyVectorTemplate(struct Command*, _CmdPtr);

// ---------------------------------------------------------------------------------------------------------------------

// REFER: https://stackoverflow.com/q/4597893
struct MyVector_pid *globalPidlist;

void handleSpawnedProcesses() {
#ifdef PRINT_DEBUG_STATEMENTS
    printflush(stderr, "DEBUG: Executing handleSpawnedProcesses(), pid=%d, ppid=%d\n", getpid(), getppid());
#endif
    for (int i = globalPidlist->n - 1; i >= 0; --i) {
        kill(globalPidlist->arr[i], 9);
        mv_pop_back_pid(globalPidlist, 1);
#ifdef PRINT_DEBUG_STATEMENTS
        printflush(stderr, "DEBUG: handleSpawnedProcesses() stopping ---> process pid=%d\n", globalPidlist->arr[i]);
#endif
    }
}

// What is the expected exit code to be returned on receiving the SIGNALS ? ---> It is said on Moodle to use exit code = signal received
void handle_signals(int signalNum) {
    // REFER: https://www.geeksforgeeks.org/signals-c-language/
    if (signalNum == SIGINT) {
        // signal 2 - Interrupt the process
        printflush(stdout, "\n%s ", str_SIGNAL_SIGINT);
        char *userChoice = inputBuffer;
        read(STDIN_FILENO, userChoice, 2048);
        // fscanf(stdin, "%[^\n]", &userChoice[0]);
        // userChoice[0] = getchar();
        // myShellInput(userChoice);
        if (userChoice[0] == 'Y') {
            handleSpawnedProcesses();
            exit(2);  // bash ---> exit(128 + 2);
        }
        // while(userChoice[0] != '\0' && userChoice[0] != '\n' && userChoice[0] != -1) {
        //     userChoice[0] = getchar();
        //     printflush(stderr, "userChoice = %d\n", userChoice[0]);
        // }
        printflush(stdout, "%s", str_myShellPrompt);
    } else if (signalNum == SIGTERM) {
        // signal 15
        printflush(stdout, "\n%s\n", str_SIGNAL_SIGTERM);
        handleSpawnedProcesses();
        exit(15);  // bash ---> exit(128 + 15);
    }
}

// ---------------------------------------------------------------------------------------------------------------------

/* Delimiters = { ' ' , '\t' , ';' , '<' , '>' , '|' } */
bool parse_isDelimiter(char ch) {
    return (ch == ' ' || ch == '\t' || ch == ';' || ch == '<' || ch == '>' || ch == '|');
}

/* Delimiters = { ';' , '|' } */
bool parse_isCommandDelimiter(char ch) {
    return (ch == ';' || ch == '|');
}

/* Delimiters = { '<' , '>' , '|' } */
bool parse_isIODelimiter(char ch) {
    return (ch == '<' || ch == '>' || ch == '|');
}

/* skip all the spaces and tabs */
int parse_skipSpacesTabs(const char *userInput, int idx) {
    while (userInput[idx] != '\0' && (userInput[idx] == ' ' || userInput[idx] == '\t')) ++idx;
    return idx;
}

/* GIVEN : Assume that the redirection symbols, < , >, >> will be delimited from
 *         other command line arguments by white space - one or more spaces.
 * NOTE  : The pointer in userInput will NOT move forward
 * */
int checkWordType(const char *userInput, int idx) {
    switch (userInput[idx]) {
        case '<':
            return 0;
        case '>':
            return ((userInput[idx + 1] == '>') ? (2) : (1));  // "1 for >" AND "2 for >>"
        case '|':
            return 7;
        case ';':
            return 15;
        case '"':
            return 20;
        case '\'':
            return 21;
    }
    return 99;
    // printflush(stderr, "\nWARNING: unhandled case ---> userInput[%d] = %c\n", idx, userInput[idx]);
}

/* If there are quotes in the input, they will be removed,
 * returns int(idx) which points the next character after the word scanned */
int parse_scanWord(const char *userInput, struct MyVector_CHAR *parsedStrings, struct Command *cmd, int idx) {
    idx = parse_skipSpacesTabs(userInput, idx);
    char quoteType = '\0';
    bool quotePresent = false;
    if (userInput[idx] == '"' || userInput[idx] == '\'') {
        quoteType = userInput[idx];
        quotePresent = true;
        ++idx;
    }

    // insert the index of the word start in "parsedStr" to the "commandIdx" vector
    const int wordStart = parsedStrings->n;
    mv_push_back_INT(&(cmd->commandIdx), wordStart);

    if (quotePresent) {
        // NOTE: once inside the quote, delimiters are of NO use
        while (userInput[idx] != '\0' && userInput[idx] != quoteType) {
            // NOTE: no special treatment for \ character, the next character is directly
            //       skipped neither is it used to allow multiline commands nor anything else
            if (userInput[idx] == '\\') {
                if (userInput[idx + 1] != quoteType)  // this is done because BASH works like this __echo "1'2\'\"3"__ ---> __1'2\'"3__
                    mv_push_back_CHAR(parsedStrings, '\\');  // userInput[idx] == '\\'
                mv_push_back_CHAR(parsedStrings, userInput[idx + 1]);
                idx += 2;
                continue;
            }
            mv_push_back_CHAR(parsedStrings, userInput[idx]);
            ++idx;
        }
        if (userInput[idx] != quoteType) printflush(stderr,
                                                    "\nERROR: quote termination not found :(\n\tidx=%d, ch=%c, userInput=```%s```",
                                                    idx, userInput[idx], userInput);
        ++idx;  // this is done to skip the last character which is quoteType, i.e. the closing quote
    } else {
        // isalnum(userInput[idx]) --- this is not used to ensure characters of
        // other language and other characters like '-' ',' '.' and many more work
        // and are NOT misinterpreted as delimiters/special characters
        while (userInput[idx] != '\0' && (!parse_isDelimiter(userInput[idx]))) {
            mv_push_back_CHAR(parsedStrings, userInput[idx]);
            ++idx;
        }
    }
    // terminate the word
    mv_push_back_CHAR(parsedStrings, '\0');

    return idx;
}

/* The returned value will be points to last subcommand's terminator */
int parse_subcommand(char *userInput, int i, struct MyVector_CHAR *parsedString, struct Command *cmd1) {
    // scan FIRST command
    i = parse_skipSpacesTabs(userInput, i);

    while (userInput[i] != '\0') {
        // NOTE: we do NOT update i over here
        int wordType = checkWordType(userInput, i);
        // printflush(stderr, "\nwordType=%d", wordType)  // REMOVE

        if (wordType == 0) {
            // input redirection <
            ++i;
            i = parse_scanWord(userInput, parsedString, cmd1, i);
            int temp_inputFileIdx = mv_pop_back_INT(&(cmd1->commandIdx), 1);  // store the index of INPUT file name character array
            mv_push_back_INT(&(cmd1->inputFileIdx), temp_inputFileIdx);
            // printflush(stderr, "\nDEBUG: temp_inputFileIdx=%d", temp_inputFileIdx);  // REMOVE
        } else if (wordType == 1) {
            // output redirection >
            ++i;
            i = parse_scanWord(userInput, parsedString, cmd1, i);
            int temp_outputFileIdx = mv_pop_back_INT(&(cmd1->commandIdx), 1);  // store the index of OUTPUT file name character array
            mv_push_back_INT(&(cmd1->outputFileIdx), -temp_outputFileIdx);  // NOTE: -ve value means that the file is overwritten
        } else if (wordType == 2) {
            // output redirection >>
            i += 2;
            i = parse_scanWord(userInput, parsedString, cmd1, i);
            int temp_outputFileIdx = mv_pop_back_INT(&(cmd1->commandIdx), 1);  // store the index of OUTPUT file name character array
            mv_push_back_INT(&(cmd1->outputFileIdx), temp_outputFileIdx);  // NOTE: +ve value means that content is appended to file
        } else if (wordType == 7) {
            // output redirection |
            cmd1->outputToPipe = true;
            // printflush(stderr, "DEBUG: -> (d3) %l, %d", cmd1, cmd1->outputToPipe);  // REMOVE
            return i;
        } else if (wordType == 15) {
            // wordType is ";"
            return i;
        } else {
            // this is VERY IMPORTANT
            i = parse_scanWord(userInput, parsedString, cmd1, i);  // NOTE: this will append the idx of the word to cmd1
        }
        // this is VERY IMPORTANT
        i = parse_skipSpacesTabs(userInput, i);
    }
    return i;
}

/* returns bool: false if not command present */
bool parse_command(char *userInput, struct MyVector_CHAR *parsedString, struct MyVector_CmdPtr *cmdlist) {
    // Clear the content of parsedString
    parsedString->n = 0;

    // Free all the command objects and clear the vector content
    for (int i = 1; i < cmdlist->n; ++i) command_free(cmdlist->arr[i]);
    cmdlist->n = 1;
    struct Command *cmd1 = cmdlist->arr[0];
    command_clear(cmd1);

    int i = 0;  // used for iterating the input

    // no command/input
    i = parse_skipSpacesTabs(userInput, i);
    if (userInput[i] == '\0') {
        return false;
    }

    i = parse_subcommand(userInput, i, parsedString, cmd1);  // scan the first command
    // printflush(stderr, "\n---> cmd1 complete=%d", i);  // REMOVE
    while (userInput[i] != '\0') {
        // REMOVE 2 lines below
        // dbi(cmdlist->n);
        // printflush(stderr, "\n-> (d1) %d, %d, %c, ", i, userInput[i], userInput[i]);

        i = parse_skipSpacesTabs(userInput, i);
        if (userInput[i] == '\0') break;
        if (userInput[i] == '|') {
            // REMOVE below debug statements
            // dbss("\npipe in cmd[n]");
            // printflush(stderr, "\nDEBUG: pipe val=%d [idx=%d] ( ptr=%l )", cmdlist->arr[cmdlist->n - 1]->outputToPipe, cmdlist->n, cmdlist->arr[cmdlist->n - 1]);

            // create a new command
            mv_push_back_CmdPtr(cmdlist, command_constructor());
            cmdlist->arr[cmdlist->n - 1]->inputFromPipe = true;  // cmdlist->arr[cmdlist->n-2]->outputToPipe;
            i += 1;
        } else if (userInput[i] == ';') {
            mv_push_back_CmdPtr(cmdlist, command_constructor());
            i += 1;
            continue;
        }
        i = parse_skipSpacesTabs(userInput, i);
        // mv_push_back_INT(&cmdlist->arr[cmdlist->n - 1]->commandIdx, i);
        // printflush(stderr, "\n-> (d2) %d, %d, %c, ", i, userInput[i], userInput[i]);  // REMOVE
        i = parse_subcommand(userInput, i, parsedString, cmdlist->arr[cmdlist->n - 1]);
    }

    // TODO: remove trailing garbage arguments - NOT sure of this

#ifdef PRINT_DEBUG_STATEMENTS
    printflush(stderr, "------------------\n\n");
    printflush(stderr, "Parsed String:");
    for (int it = 0, toprint = 1; it < parsedString->n; ++it) {
        if (toprint) {
            printflush(stderr, "[%d]%s,", it, &parsedString->arr[it]);
        }
        toprint = (parsedString->arr[it] == '\0');
    }
    for (int ite = 0; ite < cmdlist->n; ++ite) {
        struct Command *cmdTemp = cmdlist->arr[ite];

        printflush(stderr, "\n\nparse_command Output %d:", ite);
        printflush(stderr, "\t(");
        for (i = 0; i < cmdTemp->commandIdx.n; ++i) {
            printflush(stderr, "[%d]%s,", cmdTemp->commandIdx.arr[i], &parsedString->arr[cmdTemp->commandIdx.arr[i]]);
        }
        printflush(stderr, ")\t(");
        for (i = 0; i < cmdTemp->inputFileIdx.n; ++i) {
            printflush(stderr, "[%d]%s,", cmdTemp->inputFileIdx.arr[i], &parsedString->arr[cmdTemp->inputFileIdx.arr[i]]);
        }
        printflush(stderr, ")\t(");
        for (i = 0; i < cmdTemp->outputFileIdx.n; ++i) {
            // NOTE: values in outputFileIdx can be -ve which means file is to be overwritten
            printflush(stderr, "[%d]%s,", cmdTemp->outputFileIdx.arr[i], &parsedString->arr[abs(cmdTemp->outputFileIdx.arr[i])]);
        }
        printflush(stderr, ")");
        printflush(stderr, "\t(%d,%d)", cmdTemp->inputFromPipe, cmdTemp->outputToPipe);
    }
    dbss("\n\n------------------\n");
#endif

    return (cmd1->commandIdx.n == 0);
}

// ---------------------------------------------------------------------------------------------------------------------

// REFER: https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c
bool fileExists(char *fileName) {
    return access(fileName, F_OK) != -1;
}

char *getCommandPath(char *cmdName) {
    static char commandPathResult[2048];
    static char envPath[2048];

    // REFER: https://stackoverflow.com/questions/21518123/how-to-get-all-environment-variables-set-by-a-c-program-itself
    // REFER: http://www.cplusplus.com/reference/cstring/strcpy/
    strcpy(envPath, getenv("PATH"));

    // Most probably this statement will never be executed because 2048 is quite large and not path to executable is so long
    if ((strlen(envPath) + strlen(cmdName) + 2) >= 2048) {
        printflush(stderr, "WARNING: commandPathResult is smaller than the requirement ---> Therefore, returning empty string without performing any check")
        commandPathResult[0] = '\0';
        return &commandPathResult[0];
    }

    // REFER: http://www.cplusplus.com/reference/cstring/strtok/
    char *pathToTest = strtok(envPath, ":");
    while (pathToTest != NULL) {
        strcpy(commandPathResult, pathToTest);
        char *end = commandPathResult;
        while (*end) ++end;
        if (end[-1] != '/') {
            *end = '/';
            ++end;
        }
        strcpy(end, cmdName);

        // REFER: https://stackoverflow.com/questions/41230547/check-if-program-is-installed-in-c
        if (access(commandPathResult, X_OK) == 0) {
            return &commandPathResult[0];
        }
        pathToTest = strtok(NULL, ":");
    }

    *commandPathResult = '\0';
    return &commandPathResult[0];  // return empty string
}

void getUserSystemTicks(char *buffer, char *procPidStat, long *userModeTicks, long *systemModeTicks) {
    char *ptrIter;

    int procPidStatFileFD = open(procPidStat, O_RDONLY);
    read(procPidStatFileFD, buffer, 1024);
    close(procPidStatFileFD);

    ptrIter = buffer;
    for (int i = 0; *ptrIter && i < 13; ++ptrIter) {
        i += ((*ptrIter) == ' ');
    }

    (*userModeTicks) = strtol(ptrIter, &ptrIter, 10);  // read the 14th value, refer man page for proc
    (*systemModeTicks) = strtol(ptrIter, &ptrIter, 10);  // read the 15th value, refer man page for proc
}

void getTotalCPUTicks(char *buffer, long *totalCpuTicks) {
    char *ptrIter;

    int procStatFileFD = open("/proc/stat", O_RDONLY);
    read(procStatFileFD, buffer, 1024);
    close(procStatFileFD);

    ptrIter = buffer;
    while ((*ptrIter) != ' ') ++ptrIter;

    *totalCpuTicks = 0;
    while ((*ptrIter) != '\n') (*totalCpuTicks) += strtol(ptrIter, &ptrIter, 10);
}

bool isCustomFunctionThenExecute(struct Command *cmd) {
    // NOTE: This function is always called in child process.
    //       So, use exit code 127 if any error occures to print "Illegal command or arguments"

    if (strcmp(cmd->command.arr[0], "checkcpupercentage") == 0) {
        // TODO: implement and confirm the formula
        if (cmd->commandIdx.n <= 1) exit(127);  // if PID is not passed, then exit with error

        char procPidStat[32] = "/proc/";
        strcpy(procPidStat + 6, cmd->command.arr[1]);
        char *ptrIter = procPidStat + 6;
        while (*ptrIter) ++ptrIter;
        strcpy(ptrIter, "/stat");

        if (!fileExists(procPidStat)) exit(127);
        char buffer[1024] = {};

        // REFER: https://stackoverflow.com/questions/1420426/how-to-calculate-the-cpu-usage-of-a-process-by-pid-in-linux-from-c/1424556#1424556
        long userModeTicks_before, systemModeTicks_before, totalCpuTicks_before;  // these value represent time spent in ticks
        long userModeTicks_after, systemModeTicks_after, totalCpuTicks_after;  // these value represent time spent in ticks
        getUserSystemTicks(buffer, procPidStat, &userModeTicks_before, &systemModeTicks_before);
        getTotalCPUTicks(buffer, &totalCpuTicks_before);
        sleep(1);
        getUserSystemTicks(buffer, procPidStat, &userModeTicks_after, &systemModeTicks_after);
        getTotalCPUTicks(buffer, &totalCpuTicks_after);

        // const long clockTicksPerSecond = sysconf(_SC_CLK_TCK);

#ifdef PRINT_DEBUG_STATEMENTS
        printflush(stderr, "\nuser, system, total = { %ld, %ld, %ld } , clk tck = %ld\n", userModeTicks_before, systemModeTicks_before, totalCpuTicks_before, sysconf(_SC_CLK_TCK))
        printflush(stderr, "\nuser, system, total = { %ld, %ld, %ld } , clk tck = %ld\n", userModeTicks_after, systemModeTicks_after, totalCpuTicks_after, sysconf(_SC_CLK_TCK))
#endif

        double userUtilization = 100.0 * (userModeTicks_after - userModeTicks_before) / (totalCpuTicks_after - totalCpuTicks_before);
        double systemUtilization = 100.0 * (systemModeTicks_after - systemModeTicks_before) / (totalCpuTicks_after - totalCpuTicks_before);
        // long userUtilization = 100 * userModeTicks_before / totalCpuTicks_before;
        // long systemUtilization = 100 * systemModeTicks_before / totalCpuTicks_before;
        printflush(stdout, "user mode cpu percentage: %.2lf%%\n", userUtilization);
        printflush(stdout, "system mode cpu percentage: %.2lf%%\n", systemUtilization)
        exit(0);
    } else if (strcmp(cmd->command.arr[0], "checkresidentmemory") == 0) {
        // ps --no-headers --format rss PID
        // REFER: https://linux.die.net/man/2/execve
        char *customCommand[] = {"ps", "--no-headers", "--format", "rss", cmd->command.arr[1], NULL};
        execve(getCommandPath(customCommand[0]), customCommand, environ);
        exit(127);
    } else if (strcmp(cmd->command.arr[0], "listFiles") == 0) {
        // Sets the file permission to -rw-r--r-- (i.e. 644)
        dup2(open("files.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH), STDOUT_FILENO);
        char *customCommand[] = {"ls", NULL};
        execvp(customCommand[0], customCommand);
        exit(127);
    } else if (strcmp(cmd->command.arr[0], "sortFile") == 0) {
        if (fileExists(cmd->command.arr[1])) {
            dup2(open(cmd->command.arr[1], O_RDONLY), STDIN_FILENO);
            char *customCommand[] = {"sort", NULL};
            execvp(customCommand[0], customCommand);
        }
        exit(127);
    } else if (strcmp(cmd->command.arr[0], "executeCommands") == 0) {
        if (fileExists(cmd->command.arr[1])) {
            dup2(open(cmd->command.arr[1], O_RDONLY), STDIN_FILENO);
            int exitCodeToUse = 0;
            myshell(false, &exitCodeToUse);
            exit(exitCodeToUse);
        }
        exit(127);
    }
    return false;  // the requested function is NOT a customFunction
}

// ---------------------------------------------------------------------------------------------------------------------

/*
 * getconf CLK_TCK - this is OS/hardware dependent. So, always use this at runtime for conversion
 *
 * Commands to test the shell working
 * $ ls | grep -i c | grep she
 * $ echo -e '203050054.tar.gz\na.out\ncmake-build-debug\nCMakeLists.txt\nREADME.md\nshell_fm.c\nshell_test.c'
 * $ echo -e '203050054.tar.gz\na.out\ncmake-build-debug\nCMakeLists.txt\nREADME.md\nshell_fm.c\nshell_test.c' | grep -i c
 * $ echo -e '203050054.tar.gz\na.out\ncmake-build-debug\nCMakeLists.txt\nREADME.md\nshell_fm.c\nshell_test.c' | grep -i c | grep she
 * $ sleep 4 ; sleep 4 ; sleep 4 ; sleep 4 ; sleep 4 ; sleep 4;
 *
 * asdf "123" '456' < in > out
 * asdf "123" '456' < in > out | second
 * asdf "1'2\'2\"3" '4"5\"6\'78' < in > out >> out2 | second < f1 < f2
 *
 * */

/* ASSUMPTIONS:
 *      I/O are to only one place/file, i.e. echo "cool" > a.txt > b.txt | grep "c"     is not handled
 *      There is NO invalid syntax in the INPUT (userInput)
 *
 * Can use this to check if command exists or not: https://stackoverflow.com/questions/41230547/check-if-program-is-installed-in-c
 *
 * Returns bool: true if the program is supposed to exit, otherwise false
 * */
bool execute_command_v1(struct MyVector_CHAR *parsedString, struct MyVector_CmdPtr *cmdlist, struct MyVector_pid *pidlist, int *exitCodeToUse) {
    int fd[2] = {-1, -1};  // used to store file descriptors for PIPE
    int commandInOut[2];  // used to change STD I/O file numbers for each process that runs
    pid_t cmdPID;  // PID of forked process and used for waiting for spawned process
    int exitCode;

    // clear pidlist before using it for each command
    pidlist->n = 0;

    // TODO: can disable interrupt handler so then when programs like Python are running and when CTRL+C is pressed, then the interrup is not caught by our program
    // NOTE: do NOT do this now because of the requirements of the assignment
    // signal(SIGINT, NULL);
    // signal(SIGTERM, NULL);
    for (int i = 0; i < cmdlist->n; ++i) {
        commandInOut[0] = STDIN_FILENO;
        commandInOut[1] = STDOUT_FILENO;

        if (cmdlist->arr[i]->inputFromPipe) {
            // input from PIPE - will replace STDIN
            commandInOut[0] = fd[0];

            // // if input and output both are to and from PIPE, then this is REQUIRED
            // if (cmdlist->arr[i]->outputToPipe) {
            //     close(fd[0]);
            //     close(fd[1]);
            //     fd[0] = fd[1] = -1;
            // }
        } else if (cmdlist->arr[i]->inputFileIdx.n > 0) {
            // input from file
            int idxToFileName = cmdlist->arr[i]->inputFileIdx.arr[0];
            if (fileExists(&parsedString->arr[idxToFileName])) {
                commandInOut[0] = open(&parsedString->arr[idxToFileName], O_RDONLY);
            } else {
                // file does NOT exist
                printflush(stdout, "%s\n", str_ILLEGAL_COMMAND);
                // the below statements ensure that the un-used end's of the pipe is closed
                if (fd[0] != -1) close(fd[0]);
                if (fd[1] != -1) close(fd[1]);
                break;  // we stop the execution process because INVALID ARGUMENT encountered
            }
        }

        if (cmdlist->arr[i]->outputToPipe) {
            // output to PIPE - will replace STDOUT
            if (pipe(fd)) {
                printflush(stderr, "ERROR: unable to initialize the PIPE\n");
                return false;  // SIGPIPE = 13
            }
            commandInOut[1] = fd[1];
        } else if (cmdlist->arr[i]->outputFileIdx.n > 0) {
            // output to file

            // NOTE: As mentioned in struct Command definition, if value an element in outputFileIdx is -ve, then overwrite the file
            int idxToFileName = cmdlist->arr[i]->outputFileIdx.arr[0];

            // NOTE: it is assumed that idxToFileName will NEVER be 0 because at 0th index the command to be executed will be stored
            if (idxToFileName < 0) {
                // Replace file content
                // REFER: https://linux.die.net/man/3/open
                // REFER: https://man7.org/linux/man-pages/man2/open.2.html#:~:text=The%20full%20list%20of%20file,as%20a%20single%20atomic%20step.
                // Flags used are -> create file, don't create if exists, write only, truncate file content
                commandInOut[1] = open(&parsedString->arr[-idxToFileName], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            } else {
                // Append to file
                // Flags used are -> create file, don't create if exists, write only, append to file content
                commandInOut[1] = open(&parsedString->arr[idxToFileName], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            }
        }

        command_initCharPtrArray(parsedString, cmdlist->arr[i]);
        // small optimization - do not fork a process if there is NO command to execute
        if (
                cmdlist->arr[i]->inputFromPipe == false &&
                cmdlist->arr[i]->outputToPipe == false &&
                cmdlist->arr[i]->commandIdx.n == 0 &&
                cmdlist->arr[i]->inputFileIdx.n == 0 &&
                cmdlist->arr[i]->outputFileIdx.n == 0
                ) {
            continue;
        }

        if (strcmp(cmdlist->arr[i]->command.arr[0], "cd") == 0) {
            // REFER: https://linux.die.net/man/3/chdir
            chdir(cmdlist->arr[i]->command.arr[1]);
            if (commandInOut[0] != -1 && commandInOut[0] != STDIN_FILENO && commandInOut[0] != STDOUT_FILENO) close(commandInOut[0]);
            if (commandInOut[1] != -1 && commandInOut[1] != STDIN_FILENO && commandInOut[1] != STDOUT_FILENO) close(commandInOut[1]);
            continue;
        }
        if (strcmp(cmdlist->arr[i]->command.arr[0], "exit\0") == 0) {
            *exitCodeToUse = 0;
            if (cmdlist->arr[i]->command.n > 2) { // NOLINT(cert-err34-c)
                // NOTE: we check command above. So, the length of vector == input parameters+1,
                //       because the last value in vector is NULL
                // try exiting with the exit code given by the user as a parameter
                // REFER: https://stackoverflow.com/questions/13145777/c-char-to-int-conversion
                // exit(atoi(&parsedString.arr[cmdlist.arr[0]->commandIdx.arr[1]]));
                *exitCodeToUse = atoi(cmdlist->arr[i]->command.arr[1]);
            }
            handleSpawnedProcesses();
            return true;
        }

        switch (cmdPID = fork()) {
            case -1: printflush(stderr, "ERROR: fork() returned -1\n");
                break;

            case 0:
                // CHILD process
                // the if condition ensure that the same file descriptor is not closed and remapped again
                if (commandInOut[0] != STDIN_FILENO) {
                    dup2(commandInOut[0], STDIN_FILENO);
                }
                if (commandInOut[1] != STDOUT_FILENO) {
                    dup2(commandInOut[1], STDOUT_FILENO);
                }

                // the below statements ensure that the un-used end's of the pipe is closed
                if (fd[0] != -1 && fd[0] != commandInOut[0] && fd[0] != commandInOut[1]) close(fd[0]);
                if (fd[1] != -1 && fd[1] != commandInOut[0] && fd[1] != commandInOut[1]) close(fd[1]);

                if (!isCustomFunctionThenExecute(cmdlist->arr[i])) {
                    execvp(cmdlist->arr[i]->command.arr[0], cmdlist->arr[i]->command.arr);
                }

                // ### NOTE: If execvp exits with an error, then it starts executing from the next line
                // ### NOTE: If successfully executes, then it does not return
                exit(127);
                // REFER: https://stackoverflow.com/questions/1101957/are-there-any-standard-exit-status-codes-in-linux
                // https://www.gnu.org/software/bash/manual/html_node/Exit-Status.html#:~:text=A%20non%2Dzero%20exit%20status%20indicates%20failure.&text=When%20a%20command%20terminates%20on,the%20return%20status%20is%20126.
                // If a command is NOT found, then exit with code 127

            default:
                // PARENT process
                mv_push_back_pid(pidlist, cmdPID);
                if (commandInOut[0] != -1 && commandInOut[0] != STDIN_FILENO && commandInOut[0] != STDOUT_FILENO) close(commandInOut[0]);
                if (commandInOut[1] != -1 && commandInOut[1] != STDIN_FILENO && commandInOut[1] != STDOUT_FILENO) close(commandInOut[1]);
        }
    }

    // NOTE: we will replace the PID with its exit code
    for (int i = pidlist->n - 1; i >= 0; --i) {
        cmdPID = pidlist->arr[i];
        if (cmdPID <= 0) printflush(stderr, "ERROR: PID of forked process is <= 0 ---> '%d'\n", cmdPID);
#ifdef PRINT_DEBUG_STATEMENTS
        printflush(stderr, "DEBUG: waiting for pid=%d\n", cmdPID);
#endif

        // https://stackoverflow.com/questions/27306764/capturing-exit-status-code-of-child-process#:~:text=1%20Answer&text=You%20can%20get%20the%20exit,WIFEXITED%20and%20WEXITSTATUS%20with%20it.&text=waitpid()%20will%20block%20until,the%20supplied%20process%20ID%20exits.
        if (waitpid(cmdPID, &exitCode, 0) == -1) {
            perror("waitpid() failed");
            // exit(EXIT_FAILURE);
        }
        mv_pop_back_pid(pidlist, 1);

        if (WIFEXITED(exitCode)) {
            int exitStatus = WEXITSTATUS(exitCode);
            // pidlist->arr[i] = exitStatus;
            if (exitStatus == 127) {
                // command not found
                printflush(stdout, "%s\n", str_ILLEGAL_COMMAND);
#ifdef PRINT_DEBUG_STATEMENTS
                printflush(stderr, "DEBUG: 3 exitStatus=%d\n", exitStatus);
#endif
                return false;

                // ALTERNATIVE SOLUTION: https://stackoverflow.com/questions/41230547/check-if-program-is-installed-in-c
            }/* else if (exitStatus != 0) {
                printflush(stdout, "%s\n", str_ILLEGAL_COMMAND);
#ifdef PRINT_DEBUG_STATEMENTS
                printflush(stderr, "DEBUG: 4 exitStatus=%d\n", exitStatus);
#endif
            }*/
        }
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------

int min(int a, int b) { return (a < b) ? a : b; }

/* This function is used to check if the custom vector works correctly or not
 * NOTE: this is compute intensive test of MyVector working
 * */
void mv_test() {
    struct MyVector_INT vvv;
    mv_init_INT(&vvv, 8192);

    for (int i = 0; i <= 13005; ++i) {
        mv_push_back_INT(&vvv, i);
        // printf("%ld, %02d, %02d, %02d, %d\n", vvv.arr, vvv.nMax, vvv.n, i, vvv.arr[i]);

        // WARNING: this loop will case the timecomplexity of the test to become N*N
        // Comment this to see the performance of push_back
        for (int j = 0; j < vvv.n; ++j) if (vvv.arr[j] != j) printflush(stderr, "\nERROR at idx=%d\n", j);
    }
    dbss("successfully tested push_back\n");

    int oldn = vvv.n, popCount;
    for (int i = 1; vvv.n > 0; i += 3) {
        popCount = min(i, vvv.n);
        mv_pop_back_INT(&vvv, popCount);
        if (vvv.n == (oldn - popCount))
            oldn = vvv.n;
    }
    dbss("successfully tested pop_back\n");

    mv_free_INT(&vvv);
}

// ---------------------------------------------------------------------------------------------------------------------

void myshell(bool showPrompt, int *exitCodeToUse) {
    char *userInput = inputBuffer;

    struct MyVector_CHAR parsedString;
    mv_init_CHAR(&parsedString, MAX_INPUT_LENGTH);

    // GIVEN: You can assume that there won’t be more than two commands executed in parallel.
    struct MyVector_CmdPtr cmdlist;
    mv_init_CmdPtr(&cmdlist, MAX_COMMANDS_LENGTH);
    mv_push_back_CmdPtr(&cmdlist, command_constructor());

    struct MyVector_pid pidlist;
    mv_init_pid(&pidlist, MAX_COMMANDS_LENGTH);
    globalPidlist = &pidlist;  // NOTE: this is necessary to handle all the spawned process when parent receives any signal

    // TODO: future scope, this vector can be used to allow multiple file redirections like --->    ls > a.txt > b.txt >> b.txt | grep ".c$"
    // Pair of 3 integer, {in/out type, in/out file name, start, end} ---> range is [ start, end )
    // If string is '\0', then PIPE is to be used for I/O operation
    // struct MyVector_INT rangeIdx;
    // mv_init_INT(&rangeIdx, 1024);
    // This vector of char* stores strings used by rangeIdx to facilitate I/O redirections
    // struct MyVector_CHARPTR fileInOut;
    // mv_init_CHARPTR(&fileInOut, 1024);

    while (1) {
        if (showPrompt) {
#ifdef PRINT_DEBUG_STATEMENTS
            {
                static int jjjj = 0;
                ++jjjj;
                printflush(stdout, "(%d) ", jjjj);
            }
#endif
            printflush(stdout, "%s", str_myShellPrompt);
        }

        pidlist.n = 0;  // clear the PID list
        userInput[0] = '\0';  // this ensures that blank lines(i.e. NO input) are ignored

        // // REFER: https://www.includehelp.com/c/c-program-to-read-string-with-spaces-using-scanf-function.aspx
        // // REFER: https://stackoverflow.com/questions/8097620/how-to-read-from-input-until-newline-is-found-using-scanf
        // fscanf(stdin, "%[^\n]", userInput);
        // // this is required to read the trailing '\n' character OR know if user has pressed CTRL+D (i.e. getchar() returns -1)
        // if (getchar() == -1 && userInput[0] == '\0') break;
        if (!myShellInput(userInput)) {
            printflush(stdout, "\n");
            break;  // NOTE: this is an all in one solution to above 5 lines, i.e. comments, fscanf and the if condition
        }
#ifdef PRINT_DEBUG_STATEMENTS
        printflush(stderr, "userInput=%s\n", userInput);
        printflush(stderr, "userInput=%d\n", userInput[0]);
#endif
        if (userInput[0] == '\0') continue;  // do NOT do anything if line is empty

        if (parse_command(userInput, &parsedString, &cmdlist))
            continue;  // input having only blank spaces and tabs are skipped

        // TODO: move this under execute_command_v1(...) and make it a build in command
        if (strcmp(&parsedString.arr[cmdlist.arr[0]->commandIdx.arr[0]], "exit\0") == 0) {
            if (cmdlist.arr[0]->commandIdx.n > 1) { // NOLINT(cert-err34-c)
                // NOTE: we check commandIdx above. So, the length of vector == input parameters
                // try exiting with the exit code given by the user as a parameter
                // REFER: https://stackoverflow.com/questions/13145777/c-char-to-int-conversion
                *exitCodeToUse = atoi(&parsedString.arr[cmdlist.arr[0]->commandIdx.arr[1]]);
            }
            break;
        }

        if (execute_command_v1(&parsedString, &cmdlist, &pidlist, exitCodeToUse)) {
            break;
        }
    }

    mv_free_CHAR(&parsedString);
    for (int i = 0; i < cmdlist.n; ++i) command_free(cmdlist.arr[i]);
    mv_free_CmdPtr(&cmdlist);
    mv_free_pid(&pidlist);
    // mv_free_INT(&rangeIdx);
    // mv_free_CHARPTR(&fileInOut);
}


// TAGS used - TODO, DEBUG, REMOVE
int main() {
    // TODO: comment this out
    // mv_test();
    // return 0;

    // TODO: uncomment the below statements
    signal(SIGINT, handle_signals);
    signal(SIGTERM, handle_signals);

    int exitCodeToUse = 0;
    myshell(true, &exitCodeToUse);

    printflush(stdout, "\n");
    return exitCodeToUse;
}

#pragma clang diagnostic pop
