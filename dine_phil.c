#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 5

#define THINKING 0
#define EATING 1
#define HUNGRY 2

struct shm{
    int state[N];
} *shared_memory;

union semun{
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
    struct seminfo  *__buf;
} arg;

void initialize_shared_memory(){
    int shmid = shmget(IPC_PRIVATE, sizeof(*shared_memory), IPC_CREAT | 0666);
    printf("Shared memory attached at %d.\n", shmid);

    if ((shared_memory = (struct shm*) shmat(shmid, NULL, 0)) != (struct shm*) -1){
        printf("shmat successfull.\n");
    }
    else printf("shmat unsuccessful.\n");
}

int main(){

    // creating a shared memory and initializing it with zero
    initialize_shared_memory();
    for (int i = 0; i < N; i++) shared_memory->state[i] = HUNGRY;

    // creating N + 1 semaphores
    int semid = semget(IPC_PRIVATE, N + 1, IPC_CREAT | 0666);
    printf("semaphores group id: %d \n", semid);
    
    // setting all semaphores to a value
    ushort semval[N + 1];
    for (int i = 0; i < N + 1; i++) semval[i] = 2;
    arg.array = semval;
    semctl(semid, 0, SETALL, arg);

    // trying out semaphore set and get
    arg.val = 1;
    semctl(semid, 1, SETVAL, arg);
    arg.array = semval;
    semctl(semid, 0, GETALL, arg);
    for (int i = 0; i < N + 1; i++) printf("%d \n", semval[i]);

    int philosophers[N];

    for (int i = 0; i < N; i++){
        int process_id = fork();
    
        if (process_id == 0){
            // success in process creation (block goes to child)
            return 0;
        }
        else if (process_id < 0){
            // error in process creation (block goes to parent)
            printf("parent[%d]: failed to create one of the child processes, aborting...\n", getpid());

            // kill all children with SIGTERM and exit with code 1
            for (int j = 0; j < i; j++) kill(philosophers[j], SIGTERM);
            return 1;
        }
        else{
            // block for parent process
            // keep track of children so they don't get lost
            philosophers[i] = process_id;
        }
    }

    return 0;
}