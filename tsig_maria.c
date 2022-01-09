#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>


#define NUM_CHILD 3 //number of child processes 
#define SLEEP_SEC 10 //seconds of the sleeping time

int child_algorithm();

// #define WITH_SIGNALS 1 //choose with signals or not 

#ifdef WITH_SIGNALS
    bool mark=false; // keyboard interrupt occurance


    static void parent_handler(int signum)
    {
        printf("parent[%d]: Received Keyboard interrupt\n", getpid()); 
        mark=true;
    }
    static void child_handler(int signum)
    {
        printf("child[%d]: Termination of the process\n", getpid());
    }

    struct sigaction s_ignore;
    struct sigaction s_default;
    struct sigaction s_interrupt;
    

    void modify_signal_handlers()
    {
        s_ignore.sa_handler = SIG_IGN; //ignore signal
        s_default.sa_handler = SIG_DFL; //signal-specific default action 
        s_interrupt.sa_handler = parent_handler;
        s_interrupt.sa_flags = SA_RESTART;

        printf("total number of signals defined: %d \n", NSIG); 
        for (int i=1;i<NSIG; i++)
        {
            if (i==SIGCHLD)
            {
                sigaction(SIGCHLD, &s_default, NULL);
            }
            else if (i==SIGINT)
            {
                sigaction(SIGINT, &s_interrupt, NULL);
            }
            else
            {
                sigaction(i, &s_ignore, NULL);

            }
        }
    }
#endif

int main()
{
    pid_t  pid;
    int    id, ecode;
    int CHILDREN[NUM_CHILD];

    pid = getpid();
    printf( "parent[%d]: main process before fork() call.\n", pid); //parent process id --> current

    #ifdef WITH_SIGNALS
        modify_signal_handlers();
    #endif

    for (int i=1; i<=NUM_CHILD; i++)
    {
        pid=getpid(); //always the parent id 
        id=fork();

        if ( id>0 ) //check if we are in the parent process
        {        
             /* parent process */
            printf( "parent[%d]: after fork() call, created child process with id=%d.\n", pid, id );
            //keep the id of the child
            CHILDREN[i-1]=id;
            sleep(1); //1 second delay between fork calls

             #ifdef WITH_SIGNALS
                if (mark) 
                {
                    printf("parent[%d]: process interrupted!\n", pid);
                    for (int j=(i-1); j>=0; j--)
                    {
                        kill(CHILDREN[j], SIGTERM);
                    }
                    break;
                }
            #endif
        }
        else if( id==0 ) //check if we are in the child process
        {     
            /* child process */
            #ifdef WITH_SIGNALS
                struct sigaction s_child;
                s_child.sa_handler = child_handler;
                sigaction(SIGINT, &s_ignore, NULL);
                sigaction(SIGTERM, &s_child, NULL);
            #endif
            
            printf( "child[%d]: of parent[%d].\n", getpid(), getppid());
            child_algorithm();
            //exit( 3 ); /* just sample value */
            return 0;
        }
        else
        {
            /* fork creation error */
            fprintf( stderr, "parent[%d]: error fork() failed.\n", pid ); //child process unsuccesfully created 
            for (int j=(i-2); j>=0; j--)
            {
                kill(CHILDREN[j], SIGTERM);
            }
            return 1; //we exit the for loop so no more CHILDS are created
        }
    }
    int num=0;
    while(wait(NULL) != -1) 
    {
        num=num+1;
    }
    printf("parent[%d]: All children (%d) have exited \n", pid, num);

    #ifdef WITH_SIGNALS
        //restore the old service handlers of all signals
        for (int i=1; i<NSIG; i++)
        {
            //restore
            sigaction(i, &s_default, NULL);
        }
    #endif
}

int child_algorithm(){
    sleep(SLEEP_SEC);
    printf("child[%d]: process completed\n", getpid());
    return 0;
}