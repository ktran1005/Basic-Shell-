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
   * You will be able to continue a stopped job in the background using the new bg command: <br />
     ```
     $ jobs
     [0] + stopped sleep 100
     [1] + stopped sleep 500
     $ bg %1
     [1] + continued sleep 500
     $ jobs
     [0] + stopped sleep 100
     [1] + running sleep 500
     $
     ```
   * You will be able to start a job in the background by ending a command with the ampersand (&) character. Doing this causes the shell to display the job number and the involved process IDs separated by spaces: <br />
     ```
     $ frame_grabber cam0 | encode -o awesome_meme.mp4 &
     [0] 3626 3627
     $ jobs
     [0] + running frame_grabber cam0 | encode -o awesome_meme.mp4 &
     $
     ```
   * You will be able to move a background or stopped job to the foreground using the new built-in fg command: <br />
     ```
     $ jobs
     [0] + running frame_grabber cam0 | encode -o awesome_meme.mp4 &
     $ fg %0
     encoding frame 42239282 [OK]
     encoding frame 42239283 [OK]
     encoding frame 42239284 [OK]
     encoding frame 42239285 [OK]
     encoding frame 42239286 [OK]^Z
     [0] + suspended frame_grabber cam0 | encode -o awesome_mem.mp4 &
     $
     ```
   * You will also be able to kill all processes associated with a job using the new built-in kill command:
     ```
     $ jobs
     [0] + running frame_grabber cam0 | encode -o awesome_meme.mp4 &
     $ kill %0
     [0] + done frame_grabber cam0 | encode -o awesome_meme.mp4 &
     $
     ```
   
    
    
   
