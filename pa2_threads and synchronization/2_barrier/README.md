# CS 744 - Assignment 2 - Threads and Synchronization

## Student Details

* Name: Fenil Mehta
* Roll Number: 203050054

## Compile and Execution Instructions/Commands

#### Part 2 - just a barrier away!

```sh
# To compile custom testcases and execute
TEST_CASE_FILE="REPLACE with test case file name.c"

gcc -O3 -o barrierN "${TEST_CASE}" barrier.c -lpthread 
./barrierN

rm -f barrierN
```


```sh
# To compile the existing testcases
make CFLAGS=-O3

# To execute all the testcases
make run

# To manually execute each testcase
# ./barrier1
# ./barrier2
# ./barrier3
# ./barrier4
# ./barrier5

# To clean the binary files
make clean
```
