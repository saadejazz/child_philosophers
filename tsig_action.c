#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>

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
    int children_pids[NUM_CHILD];

    #ifdef WITH_SIGNALS
        // ignore all signals
        struct sigaction info;
        info.sa_handler = SIG_IGN;
        for (int j = 1; j < NSIG; j++) sigaction(j, &info, NULL);

        info.sa_handler = SIG_DFL;
        // immediately restore signal handlers as needed
        sigaction(SIGCHLD, &info, NULL);
        info.sa_handler = interrupt;
        info.sa_flags = SA_RESTART;
        sigaction(SIGINT, &info, NULL);
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
            printf("parent[%d]: failed to create one of the child processes, aborting...\n", getpid());

            // kill all children with SIGTERM and exit with code 1
            for (int j = 0; j < i; j++) kill(children_pids[j], SIGTERM);
            return 1;
        }
        else{
            // block for parent process
            // keep track of children so they don't get lost
            children_pids[i] = process_id;

            // sleep after creation of each child
            sleep(1);

            #ifdef WITH_SIGNALS
                // check if interrupted
                if (interrupted){
                    // kill all children and leave
                    for (int j = 0; j <= i; j++) kill(children_pids[j], SIGTERM);
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
    while (wait(NULL) != -1){
        num_terminations++;
    } 
    
    printf("parent[%d]: there are no more child processes.\n", c_id);
    printf("parent[%d]: received %d exit codes.\n", c_id, num_terminations);

    // #ifdef WITH_SIGNALS
    //     // restoring default service handlers of all the signals
    //     for (int i = 1; i < NSIG; i++) signal(i, SIG_DFL);
    // #endif

    return 0;
}

int child_labour(){ 
    #ifdef WITH_SIGNALS
        // signal handlers for termintaion and interrupt
        struct sigaction info;
        info.sa_handler = SIG_IGN;
        sigaction(SIGINT, &info, NULL);
        info.sa_handler = terminate_self;
        sigaction(SIGTERM, &info, NULL);
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