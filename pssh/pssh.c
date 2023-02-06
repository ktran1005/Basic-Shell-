#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "builtin.h"
#include "parse.h"

/*******************************************
 * Set to 1 to view the command line parse *
 *******************************************/
#define DEBUG_PARSE 0


void print_banner ()
{
    printf ("                    ________   \n");
    printf ("_________________________  /_  \n");
    printf ("___  __ \\_  ___/_  ___/_  __ \\ \n");
    printf ("__  /_/ /(__  )_(__  )_  / / / \n");
    printf ("_  .___//____/ /____/ /_/ /_/  \n");
    printf ("/_/ Type 'exit' or ctrl+c to quit\n\n");
}


/* returns a string for building the prompt
 *
 * Note:
 *   If you modify this function to return a string on the heap,
 *   be sure to free() it later when appropirate!  */
static char* build_prompt ()
{
    char *full_path;
    char prompt[] = "$ ";
    char *current_dir = getcwd(NULL, 0);
    full_path = malloc(strlen(current_dir) + strlen(prompt) + 1);
    strcpy(full_path, current_dir);
    strcat(full_path, prompt);
    free(current_dir);
    
    return full_path;
    
}


/* return true if command is found, either:
 *   - a valid fully qualified path was supplied to an existing file
 *   - the executable file was found in the system's PATH
 * false is returned otherwise */
static int command_found (const char* cmd)
{
    char* dir;
    char* tmp;
    char* PATH;
    char* state;
    char probe[PATH_MAX];

    int ret = 0;

    if (access (cmd, X_OK) == 0)
        return 1;

    PATH = strdup (getenv("PATH"));

    for (tmp=PATH; ; tmp=NULL) {
        dir = strtok_r (tmp, ":", &state);
        if (!dir)
            break;

        strncpy (probe, dir, PATH_MAX-1);
        strncat (probe, "/", PATH_MAX-1);
        strncat (probe, cmd, PATH_MAX-1);

        if (access (probe, X_OK) == 0) {
            ret = 1;
            break;
        }
    }

    free (PATH);
    return ret;
}


/* Called upon receiving a successful parse.
 * This function is responsible for cycling through the
 * tasks, and forking, executing, etc as necessary to get
 * the job done! */
void execute_tasks (Parse* P)
{
    // //Task *T;
    unsigned int t;
    int fd[2];
    pid_t *pid;
    pid = malloc(P->ntasks * sizeof(*pid));
    int inFile, outFile;

    if (P->infile)
    {
        inFile = open(P->infile, O_RDONLY);
    }
    else
    {
        inFile = STDIN_FILENO;
    }
    

    for (t=0; t <P->ntasks - 1; t++){
        if (pipe(fd) == -1){
            fprintf(stderr, "failed to create pipe\n");
            exit (EXIT_FAILURE);
        }
        pid[t] = fork();
        switch (pid[t]){
            case -1:
                fprintf(stderr, "failed to fork\n");
                exit(EXIT_FAILURE);
        
            case 0:
                
                close(fd[0]);

                if (inFile != STDIN_FILENO){
                    if (dup2(inFile, STDIN_FILENO) == -1){
                        fprintf(stderr, "dup2() failed 1\n");
                        exit(EXIT_FAILURE);
                    }
                close(inFile);
                }
                
                
                if(fd[1] != STDOUT_FILENO){
                    if (dup2(fd[1], STDOUT_FILENO) == -1){
                        fprintf(stderr, "dup2() failed 2\n");
                        exit(EXIT_FAILURE);
                    }
                
                }

                execvp(P->tasks[t].cmd, P->tasks[t].argv);

            default:
                break;
                        
                }
        close(fd[1]);
        if ((inFile != STDIN_FILENO)){
            close(inFile);
            }
        inFile = fd[0];

    }

    if (P->outfile){
        outFile = open(P->outfile, O_CREAT | O_WRONLY, 0644);
    }
    else{
        outFile = STDOUT_FILENO;    
    }


    
    if (!strcmp(P->tasks[t].cmd, "exit")){
        exit(EXIT_SUCCESS);
    }

    pid[t] = fork();
    switch (pid[t]){
    
    case -1:
        fprintf(stderr, "failed to fork\n");
        exit(EXIT_FAILURE);

    case 0:
        if (inFile != STDIN_FILENO){
            if (dup2(inFile, STDIN_FILENO) == -1){
                fprintf(stderr, "dup2() failed 3\n");
                exit(EXIT_FAILURE);
            }
            close(inFile);
        }
        

        if (outFile != STDOUT_FILENO){
            if (dup2(outFile, STDOUT_FILENO) == -1){
                fprintf(stderr, "dup2() failed 4\n");
                exit(EXIT_FAILURE);
            }
            close(outFile);
        }

        if (is_builtin(P->tasks[t].cmd)){
            builtin_execute(P->tasks[t]);
        }
        
        else if (command_found(P->tasks[t].cmd))
        {
            execvp(P->tasks[t].cmd, P->tasks[t].argv);
        }
        
        else
        {
            printf("pssh: command not found: %s\n", P->tasks[t].cmd);
        }


    
    default:
        break;
    }   

    for (t=0; t<P->ntasks;t++){
        wait(NULL);
    }
    free(pid);
}

int main (int argc, char** argv)
{
    char* cmdline;
    Parse* P;

    print_banner ();

    while (1) {
        cmdline = readline (build_prompt());
        if (!cmdline)       /* EOF (ex: ctrl-d) */
            exit (EXIT_SUCCESS);

        P = parse_cmdline (cmdline);
        if (!P)
            goto next;

        if (P->invalid_syntax) {
            printf ("pssh: invalid syntax\n");
            goto next;
        }

#if DEBUG_PARSE
        parse_debug (P);
#endif

        execute_tasks (P);

    next:
        parse_destroy (&P);
        free(cmdline);
    }
}
