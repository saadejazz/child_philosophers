#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int child_labour();

int main(){
    // creating a single child process
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
        wait(NULL);
        printf("parent[%d]: all children processes completed.\n", getpid());
    }

    return 0;
}

int child_labour(){
    int c_id = getpid();
    printf("child[%d]: process with parent PID %d started.\n", c_id, getppid());
    sleep(5);
    printf("child[%d]: process completed.\n", c_id);
    return 0;
}