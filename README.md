# Basic Shell

## Overview
In this project, I developed a user shell (i.e command line interface) similar to bash, zsh. This shell is able to to perform the following operations: <br />
  * Display the current working directory within the prompt (before the dollar sign): <br />
    ```
    **/home/tran/pssh$**
    ```
  * Run a single command with optional input and output redirection. Command line arguments must be supported. For example:
    ```
    **/home/tran/pssh$ ./my_prog arg1 arg2 arg3**
    **/home/tran/pssh$ ls -l > directory_contents.txt**
    **/home/tran/pssh$ grep -n "test" < input.txt > output.txt**
    ```
