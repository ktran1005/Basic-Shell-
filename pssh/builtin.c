#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "builtin.h"
#include "parse.h"

static char* builtin[] = {
    "exit",   /* exits the shell */
    "which",  /* displays full path to command */
    "jobs",
    "fg",
    "bg",
    "kill",
    NULL
};


int is_builtin (char* cmd)
{
    int i;

    for (i=0; builtin[i]; i++) {
        if (!strcmp (cmd, builtin[i]))
            return 1;
    }

    return 0;
}


void builtin_execute (Task T)
{
    if (!strcmp (T.cmd, "exit")) {
         exit (EXIT_SUCCESS);
    }

    else if(access(T.argv[1], F_OK) == 0){
        fprintf(stdout,"%s\n", T.argv[1]);
        exit(EXIT_SUCCESS);
    }
    else if (!strcmp(T.cmd, "which")){
        char *dir;
        char *tmp;
        char *PATH;
        char *state;
        char probe[PATH_MAX];
        //int ret=0;
        if (is_builtin(T.argv[1]))
        {
            printf("%s: shell built-in command\n",T.argv[1]);
        }

        else{
        PATH = strdup(getenv("PATH"));

        for (tmp=PATH; ;tmp=NULL){
            dir = strtok_r(tmp, ":", &state);
            if (!dir)
                break;
        
            strncpy(probe, dir, PATH_MAX-1);
            strncat(probe, "/", PATH_MAX-1);
            strncat(probe, T.argv[1], PATH_MAX-1);

            if (access(probe, X_OK) == 0){
                printf("%s\n", probe);
                //ret = 1;
                break;
            }
        }
        
        free(PATH);
        }
        exit(EXIT_SUCCESS);
        

    }
    else {
        printf ("pssh: builtin command: %s (not implemented!)\n", T.cmd);
    }
}
