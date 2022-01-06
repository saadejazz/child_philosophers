#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>

#define NUM_CHILD 5

int child_labour();

#ifdef WITH_SIGNALS
    // prototypes for signal handlers
    void interrupt();
    void terminate_self();

    // mark for keyboard interrupt
    bool interrupted = false;
#endif

int main(){
    int num_generated = 0;
    int children_pids[NUM_CHILD];
    
    #ifdef WITH_SIGNALS
        bool first_time = true;
    #endif

    for (int i = 0; i < NUM_CHILD; i++){
        int process_id = fork();
    
        if (process_id == 0){
            // success in process creation (block goes to child)
            printf("child[%d]: process successfully created.\n", getpid());
            return child_labour();
        }
        else if (process_id < 0){
            // error in process creation (block goes to parent)
            int c_id = getpid();
            printf("parent[%d]: failed to create one child process, aborting...\n", getpid());

            // kill all children with SIGTERM and exit with code 1
            for (int i = 0; i < num_generated; i++) kill(children_pids[i], SIGTERM);
            return 1;
        }
        else{
            // block for parent process
            #ifdef WITH_SIGNALS
                // code that only runs once
                if (first_time){
                    // ignore all signals
                    for (int i = 0; i < NSIG; i++) signal(i, SIG_IGN);

                    // immediately restore signal handlers as needed
                    signal(SIGCHLD, SIG_DFL);
                    signal(SIGINT, interrupt);

                    first_time = false;
                }
            #endif

            // keep track of children so they don't get lost
            children_pids[num_generated++] = process_id;

            // sleep after creation of each child
            sleep(1);

            #ifdef WITH_SIGNALS
                // check if interrupted
                if (interrupted) {
                    // kill all children and leave
                    for (int i = 0; i < num_generated; i++) kill(children_pids[i], SIGTERM);
                    break;
                }
            #endif
        }
    }

    int c_id = getpid();

    #ifdef WITH_SIGNALS
        // notify complete creation or interruption
        if (interrupted) printf("parent[%d]: child creation process interrupted.\n", c_id);
        else printf("parent[%d]: all children processes created.\n", c_id);
    #else
        printf("parent[%d]: all children processes created.\n", c_id);
    #endif


    // wait for all children to exit, then notify and exit.
    int num_terminations = 0;
    while (wait(NULL) != -1) num_terminations++;
    printf("parent[%d]: there are no more child processes.\n", c_id);
    printf("parent[%d]: received %d exit codes.\n", c_id, num_terminations);

    #ifdef WITH_SIGNALS
        // restoring default service handlers of all the signals
        for (int i = 0; i < NSIG; i++) signal(i, SIG_DFL);
    #endif

    return 0;
}

int child_labour(){ 
    #ifdef WITH_SIGNALS
        // signal handlers for termintaion and interrupt
        signal(SIGINT, SIG_IGN);
        signal(SIGTERM, terminate_self);
    #endif

    // the process itself
    int c_id = getpid();
    printf("child[%d]: process with parent PID %d started.\n", c_id, getppid());
    sleep(10);
    printf("child[%d]: execution complete.\n", c_id);

    return 0;
}

#ifdef WITH_SIGNALS
    void interrupt(){
        // signal handler for keyboard interrupt (SIGINT) for parent process
        printf("\nparent[%d]: received an interrupt signal.\n", getpid());
        interrupted = true;
    }

    void terminate_self(){
        // signal handler for termination signal (SIGTERM) for child process
        printf("child[%d]: received a SIGTERM, terminating...\n", getpid());
        _exit(1);
    }
#endif