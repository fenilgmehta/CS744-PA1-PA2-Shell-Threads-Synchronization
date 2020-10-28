# Shell

<!-- ----------------------------------------------------------------------------------- -->

## Usage

#### How to Compile
```sh
# make sure the Makefile is in the working directory
make
```

#### How to Run
```sh
# First compile and run the below command
./shell_fm
```

---

#### Debugging
```sh
make all_dev
./shell_fm_dev
make clean
```

<!-- ----------------------------------------------------------------------------------- -->

## Supported Commands and Special Symbols

- File redirection `<`, `>` and `>>`
- Pipe `|` - any number of times
- Quoted parameters (`'` and `"`)
- Parallel execution of commands using semicolon `;`
- `cd` - to change the current working directory
- `sortFile` - to sort the file passed to it and print to stdout
- `listFiles` - equivalent to `ls -1 > files.txt`
- `checkcpupercentage PID` - check the current CPU usage of the process with the given PID
- `checkresidentmemory PID` - check the current memory usage of the process with the given PID
- `exit` - to stop the execution of the shell
- `CTRL+C` - ask the user if they want to exit the shell or continue using it
- `executeCommands` - pass a text file as a command line parameter having a list of commands(new line separated) supported by this shell which are to be executed in a top down fashion
- **NOTE:** if a wrong command is there in the file used for `executeCommands` command, then some unknown problem happens and the project re-executes the after the line having the wrong command. See the example of wrong command below:
  ```
  echo -e '$ wrongCommand "Execution continues from the file even if wrong command is encountered"\n'
  wrongCommand "Execution continues from the file even if wrong command is encountered"
  ```
<!-- 
# TODO: add the below content to the code

# REFER
# https://www.computerhope.com/unix/signals.htm
# https://stackoverflow.com/questions/4597893/specifically-how-does-fork-handle-dynamically-allocated-memory-from-malloc/

-->

<!-- ----------------------------------------------------------------------------------- -->

## Example code snippets for testing

```sh
echo -e "a=1\nfor i in range(1000000000):\n  a += i\n  a -= i\n  a += i\n" | python ; sleep 1 | ps -a | grep python | awk '{print $1}' | xargs -I{} kill -9 {}
echo -e "print(2**100000000000)" | python ; sleep 5 | ps -a | grep python | awk '{print $1}' | xargs -I{} kill -9 {}
echo -e "print(2**100000000000)" | python ; sleep 5 | ps -a | grep python | awk '{print $1}' | xargs -I{} checkcpupercentage {}
echo -e "a=1\nfor i in range(1000000000):\n  a += i\n  a -= i\n  a += i\n" > test4.py
python test4.py

# caution "ps -a" will only list process which are running in the same terminal window
ps -a | grep python | awk '{print $1}'
ps -a | grep python | awk '{print $1}' | xargs -I{} kill -9 {}
```

```sh
# Use this to get the PID of compute and memory intensive process/PID
top | head
checkcpupercentage PID
checkresidentmemory PID
```

```sh
ls ; pwd ; echo 6 ; sleep 2 ; sleep 2 ; sleep 2 ; sleep 2 ; sleep 2 ; sleep 2
```

```sh
executeCommands z_myShellCommands.txt
```

```sh
echo -e "2\n6\n3\n1\n123\n9\n0\n4\n5" > test1.txt
cat test1.txt
sortFile test1.txt

sort -r < test1.txt > test2.txt

sort < test1.txt | grep "[3-5]" > test3.txt
cat test3.txt

cat test3.txt
echo "-----"
cat test1.txt test2.txt >> test3.txt
echo "-----"
cat test3.txt

rm test1.txt test2.txt test3.txt

ls ; exit ; pwd
```
