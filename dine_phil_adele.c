
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>

#define EXIT_SUCCESS 0
#define NUM_PHILOSOPHER 5
#define SECONDS 10 //sleeping time
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define N 5
#define LEFT (i+N-1)%N
#define RIGHT (i+1)%N

void up ();
void down();
void philosopher(int i);
void eat();
void think();
void take_forks();
void put_forks();
void test();



/* define 'state [N]' */
struct shm{
int state[N];
} *shared_memory;


/* shared memory ID */
int shmid;
union semun {
    int val; /* Value for SETVAL */
    struct semid_ds *buf; /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO
    (Linux-specific) */
} arg;


void Initialize_shared_memory(){
    /* to create share memory section*/
    int shmid = shmget (IPC_PRIVATE, sizeof(*shared_memory), IPC_CREAT | 0666); // shmgetreturns the identifier of a shared memorysegment associated with the value of the argument key
    if (shmid <0) {
        printf("error\n");
        exit(1);
    }
    else {
        printf("Memory attached at shmid [%d]\n", shmid);
    }
    shared_memory = (struct shm*) shmat(shmid, NULL, 0); //To make a requested shared memorysegment part of the address space of a process.
    /* store the shared memory Id in shmid */
    if ( shared_memory < 0) {
        printf("error\n");
        exit(1);
    }
    else{
        printf("Shmat succeed\n");
        }
}



int sem_group;


/* inizialize semaphores */
void Initialize_semaphores(){
    ushort newarray[N+1];
    sem_group = semget(IPC_PRIVATE, N+1, IPC_CREAT | 0666 );// I want a semaphore fo each philosopher , +1 is for mutex
    if ( sem_group < 0) {
        printf("error\n");
        exit(1);
    }
    else{
        printf("Semaphores group is a group id: [%d] \n", sem_group);
    }

    for(int i = 0; i < N+1; i++) {
        newarray[i] = 0;
     }
     newarray[N] = 0;
     arg.array = newarray;
   
    semctl(sem_group,0, SETALL, arg );
}


int philosopher_labour();
int p_id;
int ph_id;


void up();
void down();

int main(){
    Initialize_shared_memory();
    
    int i;
    pid_t pid[NUM_PHILOSOPHER];
    int id, ecode;
    id = getpid();
    
    for (i=0; i<NUM_PHILOSOPHER; i++){
        shared_memory->state[i]= HUNGRY;
        pid[i]=fork();
        if (pid[i] == 0) {
             /* philosopher process */
             p_id = getppid();
             ph_id = getpid();
             printf("parent[%d]: philosopher process created with success with PID:[%d].\n", p_id, ph_id );
             
             philosopher(i);

             return 0;
        }
        else if (pid[i] < 0){
            p_id = getppid();
            printf("parent [%d]:failed to create a philosopher process.\n", p_id );
            for ( int j=0;j<NUM_PHILOSOPHER; j++) kill(pid[j],SIGTERM);
            exit(EXIT_FAILURE);
            }
            else {
                /* parent process */
                ph_id = getpid();
                sleep(1);
                }
    }
    int count=0;
    while (wait(NULL) != -1){
        count++;
    }
    printf("[%d] of philosopher process exited.\n",count);
    printf( "there are no more philosopher processes.\n");
    return 0;
}


/* to implement 'up' and  'dowm' function*/
 //struct used for semaphores operations

void up (int n){
    struct  sembuf  my_sem_b;
    //operation flag set to 0
    my_sem_b.sem_num = n;//semaphore number 
    my_sem_b.sem_op= +1; //semaphore operation(add, remove )
    my_sem_b.sem_flg = SEM_UNDO;//operation flag set to 0
    
    semop(shmid, &my_sem_b, 1);
    if (shmid < 0){
    fprintf(stderr, "semaphore failde \n");
    exit(1);
    }
};


void down (int n){
    struct  sembuf  my_sem_b;
    //operation flag set to 0
    my_sem_b.sem_num = n;//semaphore number 
    my_sem_b.sem_op= -1; //semaphore operation(add, remove )
    my_sem_b.sem_flg = SEM_UNDO;//operation flag 
    
    semop(shmid, &my_sem_b, 1);
    if (shmid < 0){
    fprintf(stderr, "semaphore failde \n");
    exit(1);
    }
};



void test (int i){
    if (shared_memory->state[i]==HUNGRY && shared_memory->state[LEFT] != EATING && shared_memory->state[RIGHT] != EATING) {
        shared_memory->state[i] = EATING;
        up(i);
    }

}

void take_forks(int i ){
    down(N);
    shared_memory->state[i] = HUNGRY;
    test(i);
    up(N);
    down(i);
}

void put_forks(int i){
    down (N);
    shared_memory->state[i] = THINKING;
    test(LEFT);
    test(RIGHT);
    up(N);
}

void philosopher(int i){
    while (1){
        think();
        take_forks(i);
        eat();
        put_forks(i);
    }
}


void think(){
    ph_id = getpid();
    sleep(1);
    printf("philosopher [%d] is thinking \n",ph_id);
}

void eat (){
    ph_id = getpid();
    sleep(5);
    printf("philosopher [%d] is eathing\n",ph_id);
}