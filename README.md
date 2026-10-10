# CS 149 - Assignment 3

## Students

- Ebsan Iqbal
- Raymond Okolo

## Description

This assignment extends our Assignment 2 programs so that every `countnames` child sends its results back to the parent `shell`, and the shell adds them up.

The `shell` program is an interactive, prompt-based shell. It prints the prompt `% ` and reads a command line such as:

```text
% ./countnames test/names1.txt test/names2.txt
```

The first token is the program to run (`./countnames`) and every token after it is an input file. For each input file the shell:

1. creates a pipe with `pipe()`,
2. creates a child process with `fork()`,
3. runs `countnames` in the child with `execl()`, passing the child's PID, the file name, and the number of the pipe's write end.

All children are created before the parent waits for any of them, so the children run in parallel and a slow file never delays a fast one.

Each `countnames` child:

1. counts the names in its file with a hash table,
2. writes its results to `PID.out` and any empty-line warnings to `PID.err` (same as Assignment 2),
3. sends every unique name and its count through its pipe to the parent. Each message is a `MessageHeader` (type and payload size) followed by one `NameCountData` struct (`name` and `count`),
4. closes its end of the pipe and exits. Exit code `0` means the file was opened and counted, exit code `1` means it could not be opened.

The parent then:

1. closes the write end of each pipe right after forking (so it can see end-of-file),
2. reads each pipe until end-of-file, adding every `(name, count)` it receives into its own hash table,
3. prints the combined totals to standard output,
4. calls `wait()` until it returns `-1`, printing whether each child terminated normally or by a signal,
5. clears its table so the next command line starts from zero, and prints the prompt again.

For example, if child 1 returns `Alexa: 1, John: 2` and child 2 returns `John: 1, Jas: 5`, the shell prints `Alexa: 1`, `John: 3`, `Jas: 5`.

The shell ends when the user types `exit` or presses Ctrl+D.

If no file name is typed after `./countnames`, one child reads names from standard input.

### Files

- `shell.c` - the interactive shell (parent)
- `countnames.c` - counts names in one file (child)
- `hashtable.h` - header-only hash table used by both programs (must be submitted with the two `.c` files)
- `test/` - the test input files

---

## Compilation

Compile both programs from the project directory:

```bash
gcc -o countnames countnames.c -Wall -Werror
gcc -o shell shell.c -Wall -Werror
```

Both commands compile without errors or warnings. No extra source file is needed; `hashtable.h` is included automatically.

---

## Running the Program

Start the shell:

```bash
./shell
```

Then type commands at the `%` prompt. Type `exit` (or press Ctrl+D) to leave the shell.

The combined totals are printed to standard output. The "Child ... terminated" messages are printed to standard error, so on a terminal they appear right after the totals. The order of those messages can differ from the order the files were given, because the children run in parallel.

---

# Provided Test Cases

Start the shell with `./shell` and run each command at the `%` prompt. For a single file, the totals printed by the shell are the same names and counts that the child writes to its `PID.out`.

## Test 1 - names1.txt

Run:

```text
% ./countnames test/names1.txt
```

Expected terminal output:

```text
Tom Wu: 3
Child <PID> terminated normally with exit code: 0
```

Expected `PID.out`:

```text
Tom Wu: 3
```

Expected `PID.err`:

```text
Warning - file test/names1.txt line 3 is empty.
```

Purpose:

This test checks repeated names, and verifies that an empty line is ignored for counting and written as a warning to the error file.

---

## Test 2 - names2.txt

Run:

```text
% ./countnames test/names2.txt
```

Expected terminal output:

```text
Jenn Xu: 2
Tom Wu: 1
Child <PID> terminated normally with exit code: 0
```

Expected `PID.out`:

```text
Jenn Xu: 2
Tom Wu: 1
```

Expected `PID.err`:

```text
(empty)
```

Purpose:

This test checks that different names are counted separately and repeated names get the correct count.

---

## Test 3 - names.txt

Run:

```text
% ./countnames test/names.txt
```

Expected terminal output:

```text
Nicky: 1
Dave Joe: 2
Yuan Cheng Chang: 3
John Smith: 1
Child <PID> terminated normally with exit code: 0
```

Expected `PID.out` (same four lines).

Expected `PID.err`:

```text
Warning - file test/names.txt line 2 is empty.
Warning - file test/names.txt line 5 is empty.
```

Purpose:

This test checks names that contain spaces, repeated names, several empty lines, and the correct line numbers in the warnings.

---

## Test 4 - namesB.txt

Run:

```text
% ./countnames test/namesB.txt
```

Expected terminal output and `PID.out`:

```text
Nicky: 1
Dave Joe: 2
Yuan Cheng Chang: 3
John Smith: 1
```

followed by one `Child <PID> terminated normally with exit code: 0` line.

Expected `PID.err`:

```text
Warning - file test/namesB.txt line 2 is empty.
Warning - file test/namesB.txt line 5 is empty.
```

Purpose:

This provides another input with duplicate names, names with spaces, and empty lines.

---

## Test 5 - names_long.txt

Run:

```text
% ./countnames test/names_long.txt
```

Expected behavior:

- The child terminates normally with exit code `0`.
- The shell prints all 101 distinct entries, and `PID.out` contains the same 101 lines.
- No empty-line warnings are expected, so `PID.err` is empty.

Important lines in the output:

```text
MARY SMITH: 1
 : 99
PATRICIA JOHNSON: 1
...
ROBIN HAYES: 1
```

Purpose:

This checks a large input file, and that the program is not limited by the number of lines. It also checks that a large number of messages travels through the pipe correctly.

---

## Test 6 - names_long_redundant.txt

Run:

```text
% ./countnames test/names_long_redundant.txt
```

Expected behavior:

- Exit code `0`.
- The shell prints 91 distinct names. `PID.out` has the same 91 lines.
- Repeated names have counts greater than 1, for example:

```text
Avery Taylor: 2
Andrew Lee: 2
Nathan Davis: 2
Jonathan Lee: 2
Avery Adams: 2
```

Expected `PID.err`:

```text
Warning - file test/names_long_redundant.txt line 97 is empty.
```

Purpose:

This checks a larger input with many names, duplicates, and one empty line.

---

## Test 7 - names_long_redundant1.txt

Run:

```text
% ./countnames test/names_long_redundant1.txt
```

Expected behavior:

- Exit code `0`.
- 40 distinct names are printed (first `MARY SMITH: 1`, last `AMANDA CARTER: 1`), and `PID.out` has the same 40 lines.

Expected `PID.err`:

```text
Warning - file test/names_long_redundant1.txt line 2 is empty.
Warning - file test/names_long_redundant1.txt line 4 is empty.
Warning - file test/names_long_redundant1.txt line 6 is empty.
Warning - file test/names_long_redundant1.txt line 8 is empty.
```

Purpose:

This checks the handling and reporting of several empty lines inside a larger file.

---

## Test 8 - names_long_redundant2.txt

Run:

```text
% ./countnames test/names_long_redundant2.txt
```

Expected behavior:

- Exit code `0`.
- 44 distinct names are printed (first `STEPHANIE MITCHELL: 1`, last `SARA A PERRY: 1`), and `PID.out` has the same 44 lines.
- No warnings, so `PID.err` is empty.

Purpose:

This checks another larger set of valid names and confirms that normal files produce no warnings.

---

## Test 9 - names_long_redundant3.txt

Run:

```text
% ./countnames test/names_long_redundant3.txt
```

Expected behavior:

- Exit code `0`.
- 41 distinct names are printed, and the repeated ones have the right counts:

```text
MARY SMITH: 2
PATRICIA JOHNSON: 2
LINDA WILLIAMS: 2
BARBARA JONES: 2
ELIZABETH BROWN: 2
```

- No warnings, so `PID.err` is empty.

Purpose:

This checks correct counting when several names are repeated in a larger file.

---

# Our Test Cases

## Test A - mytestA2.txt (same file twice)

Create the file `test/mytestA2.txt` with:

```text
Alice
Bob
Alice
Charlie
Bob
Alice
```

Run the same file twice in one command:

```text
% ./countnames test/mytestA2.txt test/mytestA2.txt
```

Expected terminal output:

```text
Alice: 6
Bob: 4
Charlie: 2
Child <PID1> terminated normally with exit code: 0
Child <PID2> terminated normally with exit code: 0
```

Two different `PID.out` files are created, and each contains:

```text
Alice: 3
Bob: 2
Charlie: 1
```

Purpose:

If the same file name appears more than once, it is processed by separate children, each writes its own `PID.out`, and the parent adds the results together.

---

## Test B - namesA.txt (two empty lines in a row)

Run:

```text
% ./countnames test/namesA.txt
```

Expected terminal output and `PID.out`:

```text
Tom Holland: 1
Zendaya: 1
Hank Hill: 1
Oprah: 1
Winston: 1
```

Expected `PID.err`:

```text
Warning - file test/namesA.txt line 4 is empty.
Warning - file test/namesA.txt line 5 is empty.
```

Purpose:

This checks that consecutive empty lines are each reported with the correct line number.

---

## Test C - namesC.txt (empty first line)

Run:

```text
% ./countnames test/namesC.txt
```

Expected terminal output and `PID.out`:

```text
Mark Ruffalo: 1
Christian Bale: 1
Obama: 1
Trump: 1
Hilary: 1
```

Expected `PID.err`:

```text
Warning - file test/namesC.txt line 1 is empty.
Warning - file test/namesC.txt line 4 is empty.
Warning - file test/namesC.txt line 7 is empty.
```

Purpose:

This checks an empty first line (line 1), and empty lines in the middle of a file.

---

# Additional Validation

## Two different files at the same time

Run:

```text
% ./countnames test/names1.txt test/names2.txt
```

Expected terminal output:

```text
Tom Wu: 4
Jenn Xu: 2
Child <PID1> terminated normally with exit code: 0
Child <PID2> terminated normally with exit code: 0
```

Purpose:

This checks the main Assignment 3 requirement: the totals from two children are combined (`Tom Wu` is 3 in `names1.txt` and 1 in `names2.txt`). The two termination lines can appear in either order because the children run in parallel.

---

## Three files at the same time

Run:

```text
% ./countnames test/names1.txt test/names2.txt test/mytestA2.txt
```

Expected terminal output:

```text
Tom Wu: 4
Jenn Xu: 2
Alice: 3
Bob: 2
Charlie: 1
Child <PID1> terminated normally with exit code: 0
Child <PID2> terminated normally with exit code: 0
Child <PID3> terminated normally with exit code: 0
```

Purpose:

This checks the maximum number of input files the assignment asks for, with three children and three pipes.

---

## Same file twice (names1.txt)

Run:

```text
% ./countnames test/names1.txt test/names1.txt
```

Expected terminal output:

```text
Tom Wu: 6
Child <PID1> terminated normally with exit code: 0
Child <PID2> terminated normally with exit code: 0
```

Each of the two children writes its own `PID.out` containing `Tom Wu: 3`, and its own `PID.err` containing:

```text
Warning - file test/names1.txt line 3 is empty.
```

Purpose:

This checks that each child still creates its own output and error files, and that the parent sums identical results from two children.

---

## Totals reset between commands

At one prompt, run two commands in a row:

```text
% ./countnames test/names1.txt
% ./countnames test/names2.txt
```

Expected terminal output:

```text
Tom Wu: 3
Child <PID> terminated normally with exit code: 0
Jenn Xu: 2
Tom Wu: 1
Child <PID> terminated normally with exit code: 0
```

Purpose:

This checks that the shell clears its table after each command, so the second command does not include `Tom Wu: 3` from the first (it would otherwise print `Tom Wu: 4`).

---

## Standard input

Run:

```text
% ./countnames
```

Then type these lines and press Ctrl+D on an empty line:

```text
michael jackson
ju vue
bob marley
ju vue
```

Expected terminal output:

```text
michael jackson: 1
ju vue: 2
bob marley: 1
Child <PID> terminated normally with exit code: 0
```

Purpose:

This checks that when no file name is given, the child reads standard input, and that its results still come back through the pipe.

---

## Piped standard input directly to countnames

Run this outside the shell:

```bash
cat test/names2.txt | ./countnames
```

Nothing is printed to the terminal. The program creates a `PID.out` containing:

```text
Jenn Xu: 2
Tom Wu: 1
```

and an empty `PID.err`.

Purpose:

This checks that `countnames` also works when run by itself, with no pipe to a parent shell (no pipe write is attempted).

---

## Invalid file name

Run:

```text
% ./countnames test/file_doesnt_exist.txt
```

Expected terminal output:

```text
error: cannot open file test/file_doesnt_exist.txt
Child <PID> terminated normally with exit code: 1
```

The shell prints no totals (the child sent no data) and the prompt returns.

Purpose:

This checks the file-open error handling, the exit code `1`, and that the parent does not hang when a child sends nothing and closes its pipe.

---

## Exiting the shell

At the prompt, type `exit` (or press Ctrl+D on an empty line). The shell ends without errors. A blank line just prints the prompt again.

Purpose:

This checks that the shell loops until the user ends it.

---

# Lessons learned

In this assignment we learned how a parent and its children can communicate through a pipe: `pipe()` has to be called before `fork()` so the child inherits both ends, and each process should close the end it does not use. We saw that the parent only reaches end-of-file when every copy of the write end is closed, so the parent must close its own write end right after forking and the child must close its write end when it is done. Otherwise the parent's `read()` loop would wait forever.

We learned that a pipe is only a stream of bytes, so a message needs a header that says what type of data follows and how many bytes it has. We also learned that `read()` can return fewer bytes than requested, so we read in a loop until a whole header or struct has arrived, and that structs sent through a pipe should not contain pointers, because addresses from one process mean nothing in another.

We learned that after `exec()` the child loses its variables but keeps its open file descriptors, so the descriptor number has to be passed to `countnames` as an argument. We also practiced sharing code between programs by putting the hash table in a header, adding an `insertCount()` function so the parent can add a whole count at once, and freeing the table after each command so every command line starts from zero.

Finally, we reinforced creating all the children before waiting, so a slow file does not delay a fast one, and using `wait()` until it returns `-1` to collect every child.

---

# References

1. SJSU CS 149 Assignment 3 instructions and course slides on `fork()`, `wait()`, `exec()`, pipes, exit codes, and signals.
2. zyBooks CS 149 C programming exercises and zyLabs.
3. Linux manual pages: [pipe(2)](https://man7.org/linux/man-pages/man2/pipe.2.html), [read(2)](https://man7.org/linux/man-pages/man2/read.2.html), [write(2)](https://man7.org/linux/man-pages/man2/write.2.html), [fork(2)](https://man7.org/linux/man-pages/man2/fork.2.html), [wait(2)](https://man7.org/linux/man-pages/man2/wait.2.html).
4. TutorialsPoint, [perror](https://www.tutorialspoint.com/c_standard_library/c_function_perror.htm).
5. Anthropic Claude, used for debugging, code review, testing guidance, and README preparation.

---

# Acknowledgements

We would like to acknowledge the CS 149 course materials, lectures, slides, zyBooks exercises, and provided sample programs. We also used Anthropic's Claude AI assistant for help with debugging, reviewing our code, and preparing the README and test cases.
