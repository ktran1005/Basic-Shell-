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
#include "job.h"
/*******************************************
 * Set to 1 to view the command line parse *
 *******************************************/
#define DEBUG_PARSE 0
#define JOB_MAX 100
int num_term = 0;

Job job_list[JOB_MAX] = {0}; //Job list for the project
int our_tty;
//int flag = 0;

/*const char *sigabbrev_np(int sig)
{
				const char *sigs[31] = {"HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "BUS", "FPE", "KILL", "USR1", "SEGV", "USR2","PIPE", "ALRM", "TERM", "STKFLT", "CHLD", "CONT", "STOP", "TSTP", "TTIN", "URG", "XCPU", "XFSZ", "VTALRM", "PROF", "WINCH", "WINCH", "IO", "PWR", "SYS"};
				if (sig > 31)
								return NULL;
				return sigs[sig-1];

}*/


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

void set_fg_pgrp(pid_t pgrp)
{
	void (*sav)(int sig);
	
	if (pgrp == 0){
		pgrp = getpgrp();
	}
	sav = signal(SIGTTOU, SIG_IGN);
	tcsetpgrp(our_tty, pgrp);
	signal(SIGTTOU,sav);

}

int get_numTasks(pid_t pid)
{
	for (int i=0; i <JOB_MAX; i++)
	{
		for( int j=0; j<job_list[i].npids;j++)
		{

			if (job_list[i].pids[j] == pid)
			{
			
				return job_list[i].npids;
			}
		}
	}
	return 0;
}

int get_idx(pid_t pid){
	int job_idx = -1;
	for (int i=0; i < JOB_MAX; i++){
		for (int j=0; j < job_list[i].npids; j++){
			if (job_list[i].pids[j] == pid){
				job_idx = i;
				return job_idx;
				
			}
		}
		
		   


	}
	return job_idx;
}



void print_pids(int idx){
	for (int i=0; i<job_list[idx].npids;i++){
		printf("%d ", job_list[idx].pids[i]);
	}
	printf("\n");
}



void handler(int sig)
{
	pid_t chld;
	int status;
	
	
	switch(sig) 
	
	{
	case SIGTTIN:
		while (tcgetpgrp(our_tty) != getpgrp())
			pause();
		break;
	
	case SIGTTOU:
		while (tcgetpgrp(our_tty) != getpgrp())
			pause();
		num_term = 0;
		break;

	case SIGCHLD:
		while ((chld = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) 
		{	

			int idx =  get_idx(chld);
			 
		
			/* DO SOMETHING */
			if (WIFSTOPPED(status)){
				if (job_list[idx].status == FG){ 
					printf("\n[%d] + suspended  %s\n ", idx, job_list[idx].name); 
					job_list[idx].status = STOPPED;
					set_fg_pgrp(getpgrp());
					
				}

				else if (job_list[idx].status == BG){
					job_list[idx].status = STOPPED;
				}			
				
				continue;
			}
		

			else if (WIFCONTINUED(status)) 
		
			{		
				/* DO SOMETHING */
				if (job_list[idx].status == BG || job_list[idx].status == STOPPED){
					
					printf("\n[%d] + continued %s\n", idx, job_list[idx].name);
					job_list[idx].status = BG;
				}
				else if (job_list[idx].status == FG){
					
					set_fg_pgrp(getpgid(chld));
				}
				continue;
			}

			else 
		
			{
				/* DO SOMETHING */

				num_term++;
				if (num_term == job_list[idx].npids && job_list[idx].status == BG){
					printf("\n[%d] + done       %s\n", idx, job_list[idx].name);
					job_list[idx].status = TERM;
					num_term = 0;
					return;
				}

				if (num_term == job_list[idx].npids){					
					job_list[idx].status = TERM;
					set_fg_pgrp(getpgrp());
					num_term = 0;
				}
				continue;

			}
		}
		break;
	
	default:
		break;
	}
}

void update_task(){
	for (int i=0;i<100; i++){
		if(job_list[i].status == TERM && job_list[i].name){
			free(job_list[i].name);
			free(job_list[i].pids);
			job_list[i].name = NULL;
		}
	}
}

int check_jobValid(int idx)
{
	if (job_list[idx].name)
		return 0;
	return 1;
}

int check_pidValid(pid_t pid)
{
	for (int i=0; i < JOB_MAX; i++)
	{
		for (int j=0 ; j < job_list[i].npids; j++)
		{
			if (job_list[i].pids[j] == pid)
			{
				
				return 0;
				
			}
		}
	}
	return 1;
	
}



/* Called upon receiving a successful parse.
 * This function is responsible for cycling through the
		job_list[i].name = cmdline;
 * tasks, and forking, executing, etc as necessary to get
 * the job done! */
void execute_tasks (Parse* P, char *cmdline)
{
    
    static int i;
    unsigned int t;
    int fd[2];
    pid_t *pid;
    pid = malloc(P->ntasks * sizeof(*pid));
    int inFile, outFile;

	int specified_signal;
	int desire_job;
	pid_t desire_pid;
    if (P->infile)
    {
        inFile = open(P->infile, O_RDONLY);
    }
    else
    {
        inFile = STDIN_FILENO;
    }
	
   if (!is_builtin(P->tasks[0].cmd)){	
		for (i=0; i< 100; i++){
			if ((!job_list[i].name) || job_list[i].status == TERM){
				job_list[i].name = cmdline;
				job_list[i].npids = P->ntasks;
				job_list[i].pids = malloc(sizeof(pid_t) * P->ntasks);
				if (P->background){
					job_list[i].status = BG;
				}	
				else {
					job_list[i].status = FG;
				} 
				break;
			}
		}
   }

   else {
	   if (!strcmp(P->tasks[0].cmd, "jobs"))
	   {
		   for (i=0; i<100;i++)
		   {
			   if (job_list[i].status == STOPPED && job_list[i].name)
				  printf("[%d] + stopped   %s\n", i, job_list[i].name);
				 
			   else if (job_list[i].status != TERM && job_list[i].name)
				  printf("[%d] + running   %s\n", i, job_list[i].name);
			}
	   }

	   else if (!strcmp(P->tasks[0].cmd, "kill"))
	   {
				int argc = 0;
				int t=0;
				while (P->tasks[0].argv[t]){
					argc += 1;
					t+= 1;
				}

				if (argc != 2 && argc != 4){
					printf("Usage: kill [-s <signal>] <pid> | %%<jobs> ...\n");
				}
				if (argc == 2)
				{

					if ((P->tasks[0].argv[1][0] == '%')) 
					{
							specified_signal = SIGTERM;
							P->tasks[0].argv[1][0] = '0';
							desire_job =atoi(P->tasks[0].argv[1]);
							if (!check_jobValid(desire_job))
							{
								kill(job_list[desire_job].pgid * (-1), specified_signal);
							}
							else
								printf("pssh: invalid job number: [job number]\n");
							
					}
					else 
					{
						specified_signal = SIGTERM;
						desire_pid = (pid_t) atoi(P->tasks[0].argv[1]);
						if (!check_pidValid(desire_pid))
						{
							kill(desire_pid, specified_signal);
						}
						else
							printf("pssh: invalid pid: [pid number]\n");
					}
				}
				
				else if (argc == 4)
				{
					if (!strcmp(P->tasks[0].argv[1], "-s"))
					{
						specified_signal = atoi(P->tasks[0].argv[2]);
						if (P->tasks[0].argv[3][0] == '%')
						{
							P->tasks[0].argv[3][0] = '0';
							desire_job = atoi(P->tasks[0].argv[3]);
					
							if (!check_jobValid(desire_job))
							{
								
								kill(job_list[desire_job].pgid * (-1), specified_signal);
							}
							else
								printf("pssh: invalid job number: [job number]\n");						
						}
					
					
					
						else 
						{
							specified_signal = atoi(P->tasks[0].argv[2]);
							desire_pid = (pid_t) atoi(P->tasks[0].argv[3]);

							if (!check_pidValid(desire_pid))
							{
								kill(desire_pid,specified_signal);
							}

							else
								printf("pssh: invalid pid: [pid number]\n");


						}
					}

				}
		}
	   else if (!strcmp(P->tasks[0].cmd, "bg"))
	   {
			int argc = 0;
			int t = 0;
			while (P->tasks[0].argv[t]){
				argc++;
				t++;
			}

			if (argc < 2)
				printf("Usage: bg %%<job number>\n");
			if (argc == 2)
			{
				P->tasks[0].argv[1][0] = '0';
				desire_job = atoi(P->tasks[0].argv[1]);
				specified_signal = SIGCONT;
				if (!check_jobValid(desire_job))
				{
					job_list[desire_job].status = BG;
					kill(job_list[desire_job].pgid * (-1), specified_signal);
				}
				else
					printf("pssh: invalid job number: [job number]\n");

			}
	   }
	   else if (!strcmp(P->tasks[0].cmd,"fg"))
	   {
			int argc = 0;
			int t = 0;

			while (P->tasks[0].argv[t])
			{
				argc++;
				t++;
			}
			if (argc < 2)
				printf("Usage: fg %%<job number>\n");
			if (argc == 2)
			{
				P->tasks[0].argv[1][0] = '0';
				desire_job = atoi(P->tasks[0].argv[1]);
				specified_signal = SIGCONT;
				if (!check_jobValid(desire_job))
				{
					job_list[desire_job].status = FG;
					kill(job_list[desire_job].pgid * (-1), specified_signal);
					set_fg_pgrp(job_list[desire_job].pgid);
					
				}
				else
					printf("pssh: invalid job number: [job number]\n");
			}
	   }

	   
					
	   else 
		   builtin_execute(P->tasks[0]);
	   return;
	} 
	
	

   for (t=0; t <P->ntasks - 1; t++){
        if (pipe(fd) == -1){
            fprintf(stderr, "failed to create pipe\n");
            exit (EXIT_FAILURE);
        
		}

		pid[t] = fork();
		setpgid(pid[t],pid[0]);
		if (!P->background)
			set_fg_pgrp(pid[0]);
	
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
					
		    job_list[i].pids[t] = pid[t];
            job_list[i].pgid = getpgid(pid[0]);
            break;
                        
        }
	close(fd[1]);
	if (inFile != STDIN_FILENO)
	{
            close(inFile);
	}
    inFile = fd[0];
	}

  if (P->outfile)
  {
        outFile = open(P->outfile, O_CREAT | O_WRONLY, 0644);
  }
    
  else
	{
		outFile = STDOUT_FILENO;    
    	
	}	

    
    pid[t] = fork();
    setpgid(pid[t],pid[0]);
    if (!P->background)
    	set_fg_pgrp(pid[0]);
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

        
        if (command_found(P->tasks[t].cmd))
        {
            execvp(P->tasks[t].cmd, P->tasks[t].argv);
        }
        
        else
        {
            printf("pssh: command not found: %s\n", P->tasks[t].cmd);
        }

		
    
    default:
        //job_list[i].pids[t] = malloc(sizeof(*pids) * npids);
	    job_list[i].pids[t] = pid[t];
        job_list[i].pgid = getpgid(pid[0]);
	    signal(SIGTTOU,handler);
	    signal(SIGCHLD,handler);
      break;
    }
    if (job_list[i].status == FG){
    	kill(getpid(), SIGTTOU);
	//for (t=0; t<P->ntasks;t++){
    //    	wait(NULL);
    //	}
    
    }

    else{
		printf("[%d] ",i);
		print_pids(i);

    }
    free(pid);
}
   

int main (int argc, char** argv)
{
    our_tty = dup(STDERR_FILENO);
    char* cmdline;
    char* name;
    Parse* P;
	



    print_banner ();
    //signal(SIGTTOU, handler);
    //signal(SIGCHLD,handler);
    while (1) {
        cmdline = readline (build_prompt());
		name = strdup(cmdline);

		


        if (!cmdline)       /* EOF (ex: ctrl-d) */
            exit (EXIT_SUCCESS);

        P = parse_cmdline (cmdline); // sleep 10 | sleep 10 | sleep 10
        if (!P)
            goto next;

        if (P->invalid_syntax) {
            printf ("pssh: invalid syntax\n");
            goto next;
        }

#if DEBUG_PARSE
        parse_debug (P);
#endif

		execute_tasks (P, name);
	//if (isatty(STDOUT_FILENO))
	//	    printf("FG after the job done: %d\n", tcgetpgrp(STDOUT_FILENO));
    
    next:
        parse_destroy (&P);
		update_task();
        free(cmdline);
    }
}
