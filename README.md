# Basic Shell

## Overview
In this project, I developed a user shell (i.e command line interface) similar to bash, zsh. There are two major parts in this project. <br />

# Part 1: This shell is able to perform the following operations: <br />
  * Display the current working directory within the prompt (before the dollar sign): <br />
    ```
    /home/tran/pssh$
    ```
  * Run a single command with optional input and output redirection. Command line arguments must be supported. For example:
    ```
    /home/tran/pssh$ ./my_prog arg1 arg2 arg3
    /home/tran/pssh$ ls -l > directory_contents.txt
    /home/tran/pssh$ grep -n "test" < input.txt > output.txt**
    ```
  * Run multiple pipelined commands with optional input and output redirection. Naturally, command line arguments to programs must still be supported. For example:
    ```
    /home/tran/pssh$ ./my_prog arg1 arg2 arg3 | wc -l > out.txt
    /home/tran/pssh$ ls -lh | awk {print $9 " is " $5}
    /home/tran/pssh$ ps aux | grep bash | grep -v grep | awk  {print $2}
    ```
  * Implement the builtin command `exit', which will terminate the shell:
    ```
    /home/tran/pssh$ exit
    ```
  * Implement the builtin command `which'. This command accepts 1 parameter (a program name), searches the system PATH for the program, and prints its full path to stdout if found (or simply nothing if it is not found). If a fully qualied path or relative path is supplied to an executable program, then that path should simply be printed to stdout. If the supplied program name is another builtin command, your shell should indicate that in a message printed to stdout. The behavior should be identical to bash's builtin which command. For example:
    ```
    /home/tran/pssh$ which ls
    /bin/ls
    /home/tran/pssh$ which lakjasdlkfjasdlkfj
    /home/trand/pssh$ which exit
    exit: shell built-in command
    /home/tran/pssh$ which which
    which: shell built-in command
    /home/tran/pssh$ which man
    /usr/bin/man
    ```
# Part 2: The shell is added Process Groups, Signals and Signal Handlers in order to implement a standard job control system <br />
   * You will be able to check the status of jobs with the new built-in jobs command <br />
   * You will be able to send signals to specic processes and job numbers from your command line using the new built-in kill command <br />
   * You will be able to suspend the foreground job by hitting Ctrl+z 
     ```
     $ sleep 100
     ^Z[0]  + suspended         sleep 100
     $ jobs
     [0]    + stopped   sleep 100
     $
     ```
   
