#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_CHILD 5
#define SLEEP_TIME 10
#define FORK_DELAY 1

int create_child();
int child_labour();
void slay_all_children();

int num_generated = 1;
int children_pids[NUM_CHILD];

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
        return child_labour();
    }
    // error in process creation
    else if (process_id < 0){
        printf("parent[%d]: failed to create a child process.\n", getpid());

        // latest procreation failed, kill all children with SIGTERM (or a G3) ;(
        slay_all_children();
        return 1;
    }
    // prints after processes all exit
    else{
        if (++num_generated <= NUM_CHILD){
            // keep track of children so they don't get lost - they are still young.
            children_pids[num_generated - 2] = process_id;
            
            // procreate again after a delay
            sleep(FORK_DELAY);
            create_child();
        }
        else{
            int c_id = getpid();
            int ecode;
            
            // store the last child
            children_pids[num_generated - 2] = process_id;
            
            // notify creation of all children
            printf("parent[%d]: all children processes created.\n", c_id);
            
            // wait for all children to exit, then notify and exit.
            int num_terminations = 0;
            for (int i = 0; i < NUM_CHILD; i ++){
                if (children_pids[i]){
                    // wait for an exit code, determine the pid of the exiting child process
                    int exit_pid = wait(&ecode);
                    printf("parent[%d]: received exit code %d from child with pid %d.\n", c_id, ecode, exit_pid);
                    num_terminations++;
                }
            }
            // print the number of termination codes received.
            printf("parent[%d]: received %d exit codes.\n", c_id, num_terminations);
            printf("parent[%d]: all children processes exited.\n", c_id);
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

void slay_all_children(){
    int wstatus;

    // kill every process with pid stored in children_pids
    for (int i = 0; i < NUM_CHILD; i++){
        if (children_pids[i]){
            int c_id = getpid();
            int child_id = children_pids[i];

            // check if kill signal was sent successfully. may not be sent in case of permission errors
            int check = kill(children_pids[i], SIGTERM);
            if (check == 0){
                printf("parent[%d]: child process with id %d sent a termination signal.\n", c_id, child_id);
                
                // check if SIGTERM resulted in the killing of the child
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
}