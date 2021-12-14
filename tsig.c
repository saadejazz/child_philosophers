#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_CHILD 7
#define SLEEP_TIME 2.5

int create_child();
int child_labour();

int num_generated = 2;

int main(){
    create_child();
    return 0;
}

int create_child(){
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
        if (num_generated <= NUM_CHILD){
            num_generated++;
            create_child();
        }
        else{
            wait(NULL);
            printf("parent[%d]: all children processes completed.\n", getpid());
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