#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_CHILD 5
#define SLEEP_TIME 2

int create_child();
int child_labour();

int num_generated = 1;
int children_pids[NUM_CHILD];
int wstatus;

int main(){
    create_child();
    return 0;
}

int create_child(){
    /*a recursive function in which only the parent procreates :| */

    pid_t process_id = fork();
    
    // success in process creation
    if (process_id == 0){
        printf("parent[%d]: child process successfully created with PID: %d.\n", getppid(), getpid());
        child_labour();
    }
    // error in process creation
    else if (process_id < 0){
        printf("parent[%d]: failed to create a child process.\n", getpid());

        // latest procreation failed, kill all children with SIGTERM (or a G3) ;(
        for (int i = 0; i < NUM_CHILD; i++){
            if (children_pids[i]){
                int c_id = getpid();
                int child_id = children_pids[i];
                int check = kill(children_pids[i], SIGTERM);
                if (check == 0){
                    printf("parent[%d]: child process with id %d sent a termination signal.\n", c_id, child_id);
                    int sig = waitpid(child_id, &wstatus, WUNTRACED);
                    if (sig){
                        if (WTERMSIG(wstatus) == SIGTERM) printf("parent[%d]: child process with id %d ended with a SIGTERM signal.\n", c_id, child_id);
                        else printf("parent[%d]: child process with id %d ended with a %d signal.\n", c_id, child_id, WTERMSIG(wstatus));
                    }
                    else printf("parent[%d]: child process with id %d was not ended with a signal.\n", c_id, child_id);
                } 
                else printf("parent[%d]: could not send termination signal to child process with id %d.\n", getpid(), child_id);
            } 
        }
        return 0;
    }
    // prints after processes all exit
    else{
        if (++num_generated <= NUM_CHILD){
            // keep track of children so they don't get lost - they are still young.
            children_pids[num_generated - 2] = process_id;
            create_child();
        }
        else{
            // store the last child
            children_pids[num_generated - 2] = process_id;
            wait(NULL);
            printf("parent[%d]: all children processes completed.\n", getpid());
        }
        return 0;
    }
}

int child_labour(){
    int c_id = getpid();
    printf("child[%d]: process with parent PID %d started.\n", c_id, getppid());
    sleep(SLEEP_TIME);
    printf("child[%d]: process completed.\n", c_id);
    return 0;
}