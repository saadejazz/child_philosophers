#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include<signal.h>

#define EXIT_SUCCESS 0
#define NUM_CHILD 5
#define SECONDS 10 //sleeping time

int child_labour();
int p_id;
int c_id;
int mark = 0;

/* Signal Handler for SIGINT */
void sigint_handler() { 
   printf("parent[%d]: keyboard interrupt \n", getpid());
   mark = 1;
}

/* Signal Handler for SIGTERM */
void sigterm_handler() {
    printf("child[%d]: the process is terminated \n", getpid());
}

int main(){
    
    pid_t pid[NUM_CHILD];
    int id, ecode;
    id = getpid();

    /* ignoring all signals */
    for (int i = 1; i < NSIG; i++){
        signal(i, SIG_IGN);
    }
    /* Set the SIGINT (Ctrl-C) signal handler to sigintHandler */
    signal(SIGCHLD, SIG_DFL);
    signal(SIGINT, sigint_handler);
            
    
    for (int i = 0; i < NUM_CHILD; i++){
        pid[i]=fork();
        if (pid[i] == 0) {
            /* child process */
            signal(SIGTERM, sigterm_handler);
            signal(SIGINT, SIG_IGN);
            p_id = getppid();
            c_id = getpid();
            printf("parent[%d]: child process created with success with PID:[%d].\n", p_id, c_id);
            child_labour();
            exit(EXIT_SUCCESS);
        }
        else if (pid[i] < 0){
            p_id = getppid();
            printf("parent[%d]:failed to create a child process.\n", p_id );
            
            for (int j = 0; j <= i; j++) kill(pid[j],SIGTERM);
            exit(EXIT_FAILURE);
        }
        else {
            /* parent process */
            c_id = getpid();
            sleep(1);
            if (mark == 1){
                for (int j = 0; j <= i; j++) kill(pid[j], SIGTERM);
                break;
            }            
        }
    }

    int count=0;
    while (wait(NULL) != -1) count++;
    printf("parent[%d]: [%d] of child process exited.\n", c_id, count);
    printf("parent[%d]: there are no more child processes.\n", c_id);

    for (int i = 1; i < NSIG; i++){
        signal(i, SIG_DFL);
    }

    return 0;
}

int child_labour(void){
    c_id = getpid();
    printf("child[%d]: process with parent PID %d started.\n",c_id, getppid());
    sleep(SECONDS);
    printf("child[%d]: process completed\n", c_id);
    return 0;
}