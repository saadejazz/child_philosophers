#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#define NUM_CHILD 4
#define SLEEP_TIME 1

int create_child();
int child_labour();

int num_generated = 1;
int children_pids[NUM_CHILD];

int main(){
    create_child();
    return 0;
}

int create_child(){
    // a recursive function in which only the parent procreates :/

    pid_t process_id = fork();
    // success in process creation
    if (process_id == 0){
        printf("parent[%d]: child process successfully created with PID: %d.\n", getppid(), getpid());
        child_labour();
    }
    // error in process creation
    else if (process_id < 0){
        printf("parent[%d]: failed to create a child process.\n", getppid());
    }
    // prints after processes all exit
    else{
        if (++num_generated <= NUM_CHILD){
            // keep track of children so they don't get lost - they are still young.
            children_pids[num_generated - 2] = process_id;

            int status;
            int k = waitpid(process_id, &status, WNOHANG);

            printf("%d\n", k);

            create_child();
        }
        else{
            // store the last child
            children_pids[num_generated - 2] = process_id;
            wait(NULL);
            printf("parent[%d]: all children processes completed.\n", getpid());
            printf("%d, %d, %d, %d\n", children_pids[0], children_pids[1], children_pids[2], children_pids[3]);
            return 0;
        }
    }
}

int child_labour(){
    int c_id = getpid();
    printf("child[%d]: process with parent PID %d started.\n", c_id, getppid());
    sleep(SLEEP_TIME);
    printf("child[%d]: process completed.\n", c_id);
    return 0;
}