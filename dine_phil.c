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

#define LEFT (i + N - 1) % N
#define RIGHT (i + 1) % N


int semid;

void up(int sem_num);
void down(int sem_num);
void man_sem(int sem_num, int add);

void eat();
void think();
void test();
void take_forks(int i);
void put_forks(int i);

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

    // creating a shared memory and initializing it with HUNGRY
    initialize_shared_memory();
    for (int i = 0; i < N; i++) shared_memory->state[i] = HUNGRY;

    // creating N + 1 semaphores
    semid = semget(IPC_PRIVATE, N + 1, IPC_CREAT | 0666);
    printf("semaphores group id: %d \n", semid);
    
    // setting all semaphores to 1
    ushort semval[N + 1];
    for (int i = 0; i < N + 1; i++) semval[i] = 1;
    arg.array = semval;
    semctl(semid, 0, SETALL, arg);

    // arg.val = 0;
    // semctl(semid, N, SETVAL, arg);

    // up(3);
    // down(2);
    // up(2);

    // arg.array = semval;
    // semctl(semid, 0, GETALL, arg);
    // for (int i = 0; i < N + 1; i++) printf("%d \n", semval[i]);

    printf("Check");
    int philosophers[N];

    for (int i = 0; i < N; i++){
        int process_id = fork();
        if (process_id == 0){
            // success in process creation (block goes to child)
            int c_id = getpid();
            printf("A philosopher is born. \n");
            while (1)
            {
                printf("philosopher[%d, %d]: started thinking. \n", c_id, i);
                think();
                printf("philosopher[%d, %d]: stopped thinking. \n", c_id, i);
                take_forks(i);
                printf("philosopher[%d, %d]: started eating. \n", c_id, i);
                eat();
                printf("philosopher[%d, %d]: stopped eating. \n", c_id, i);
                put_forks(i);
            }
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
    wait(NULL);
    return 0;
}


void up(int sem_num){
    man_sem(sem_num, 1);
}

void down(int sem_num){
    man_sem(sem_num, -1);
}

void man_sem(int sem_num, int add){
    struct sembuf sems;
    sems.sem_num = sem_num;
    sems.sem_op = add;
    int ecode = semop(semid, &sems, 1);
    if (ecode == -1) fprintf(stderr, "semaphore failed.\n");
}

void eat(){
    // munching
    sleep(1);
}

void think(){
    sleep(2);
}

void take_forks(int i){
    down(N);
    shared_memory->state[i] == HUNGRY;
    test(i);
    up(N);
    down(i);
}

void put_forks(int i){
    down(N);
    shared_memory->state[i] = THINKING;
    test(LEFT);
    test(RIGHT);
    up(N);
}

void test(int i){
    if (shared_memory->state[i] == HUNGRY &&
        shared_memory->state[LEFT] != HUNGRY &&
        shared_memory->state[RIGHT] != EATING){
            shared_memory->state[i] = EATING;
            up(i);
        }
}