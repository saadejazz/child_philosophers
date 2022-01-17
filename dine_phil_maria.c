#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <sys/ipc.h>

#define N 5
#define THINKING 0
#define EATING 1  
#define HUNGRY 2 

#define LEFT (i+N-1) %N /////////////////////////////////
#define RIGHT (i+1) %N //////////////////////////////////


int PHILOSOPHERS[N];
int sem_group;

struct shm{
    int state[N]; //to use in shared memory    
} *shared_memory;

union semun{
    int val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array;  /* Array for GETALL, SETALL */
    struct seminfo *__buf;  /* Buffer for IPC_INFO
    (Linux-specific) */
} arg;

int create_philosophers(int i);
void philosopher (int i); //////////////////////////////////
void take_forks(int i);
void put_forks(int i);
void test(int i); 
void think();
void eat();/////////////////////////////////////////

void initialize_shared_memory(){
    //key_t SH;
    int shmid=shmget(IPC_PRIVATE, sizeof(*shared_memory), IPC_CREAT | 0666);
    //int shmget(key_t key, size_t size, int shmflg);
    if (shmid<0)
    {
        printf("*** shmget error ***\n");
        exit(1);
    }
    else{
        printf("Memory attached at shmid %d\n", shmid);
        shared_memory = (struct shm*) shmat(shmid, NULL, 0);
        printf("Shmat succeed\n");
    }
}

void initialize_semaphores(){

    sem_group=semget(IPC_PRIVATE, N+1, IPC_CREAT | 0666); 
    //int semget(key_t key, int nsems, int semflg);

    printf("Semaphores group id: %d \n", sem_group);

    ushort aux[N+1];
    for (int j=0; j<N+1; j++){
            aux[j]=1; //initialize to 1
    }
    arg.array=aux;

    semctl(sem_group, 0, SETALL, arg); 
    //int semctl(int semid, int semnum, int cmd, ...);
}

void up (int i){
    struct sembuf my_sem_b;
    my_sem_b.sem_num = i;
    my_sem_b.sem_op= 1; //Semaphore up
    my_sem_b.sem_flg = SEM_UNDO;

    int semop1=semop(sem_group, &my_sem_b, 1);
    //int semop(int semid, struct sembuf *sops, size_t nsops);
    if (semop1<0){
        fprintf(stderr, "semaphore failed! \n");
    } 
}

void down(int i){
    struct sembuf my_sem_b;
    my_sem_b.sem_num = i;
    my_sem_b.sem_op= -1; //Semaphore down
    my_sem_b.sem_flg = SEM_UNDO;

    int semop1=semop(sem_group, &my_sem_b, 1);
    //int semop(int semid, struct sembuf *sops, size_t nsops);

    if (semop1<0){
        fprintf(stderr, "semaphore failed! \n");
    }    
}

int main()
{
    pid_t  pid;
    int    id, ecode;
    initialize_shared_memory(); 
    initialize_semaphores();

  for (int i=0; i<N; i++)
    {
        shared_memory -> state[i]= HUNGRY;//make them hungry 
        pid=getpid(); //always the parent id 
        // sleep(1); //1 second delay between fork calls
        id=fork();

        if ( id>0 ) //check if we are in the parent process
        {        
             /* parent process */
            printf( "parent[%d]: after fork() call, created child process with id=%d.\n", pid, id );

            //keep the id of the philosophers
            PHILOSOPHERS[i]=id;
        }
        else if( id==0 ) //check if we are in the child process
        {     
            /* child process */
            printf( "philosopher[%d]: of parent[%d].\n", getpid(), getppid());
            create_philosophers(i);
            return 0;
        }
        else
        {
            /* fork creation error */
            fprintf( stderr, "parent[%d] error: fork() failed.\n", pid ); //child process unsuccesfully created 
            for (int j=i; j>=0; j--)
            {
                if (PHILOSOPHERS[j]!=0) kill(PHILOSOPHERS[j], SIGTERM);
            }
            return 1; //we exit the for loop so no more CHILDS are created
        }
    }
    wait(NULL);
}

int create_philosophers(int i){
    printf("philosopher[%d]: child created\n", getpid());
    philosopher(i);
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void philosopher(int i){
    while(1){
        think();
        take_forks(i);
        eat();
        put_forks(i);
    }
}

void think(){
    sleep(1);
    printf("philosopher[%d]: is thinking\n", getpid());
}

void eat(){
    sleep(1);
    printf("philosopher[%d]: is eating\n", getpid());
}

void take_forks(int i){
    down(N);
    shared_memory -> state[i]=HUNGRY;
    test(i);
    up(N);
    down(i);
}

void put_forks(int i){
    down(N);
    shared_memory -> state[i]=THINKING;
    test(LEFT);
    test(RIGHT);
    up(N);
}
void test(int i){
    if( shared_memory -> state[i]==HUNGRY && shared_memory -> state[LEFT]!=EATING && shared_memory -> state[RIGHT]!=EATING){
        shared_memory -> state[i]=EATING;
        up(i);
    }
}