#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>

#define NUM_CHILD 5

int child_task();
int make_child();
int pids[NUM_CHILD];

#ifdef WITH_SIGNALS

bool mark = false;

void sig_keyboard_int()
{
	printf("\nSomeone broke the living room window!\n");
	mark = true;
}
void sig_term()
{
	printf("Child [%d]: Why did you have to kill me!!!\n", getpid());
	exit(EXIT_FAILURE);
}
#endif

int main()
{
	int i;

#ifdef WITH_SIGNALS
	for (i = 1; i <= NSIG; i++)
	{
		signal(i, SIG_IGN); //ignore all signals
	}

	signal(SIGCHLD, SIG_DFL);
	signal(SIGINT, sig_keyboard_int);

#endif

	make_child();

	printf("These are the child processes created:");
	for (int i = 0; i < NUM_CHILD; i++)
	{
		printf(" [%d]", pids[i]);
	}
	printf("\n");

#ifdef WITH_SIGNALS
	for (i = 1; i <= 6; i++)
	{
		signal(i, SIG_DFL); //restore signal's default values
	}
#endif
	return 0;
}

int make_child()
{
	pid_t pid;
	int id;
	pid = getpid();
	int n_childs = 0;

	for (int i = 0; i < NUM_CHILD; i++)
	{
		
#ifdef WITH_SIGNALS

    if (mark == true)
    {
        for (int k = 0; k <= i; k++)
        {
            if (pids[k] != 0)
            {
                kill(pids[k], SIGTERM);
                printf("Parent: You won't see light of another day!\n");
            }
        }
        break;
    }
#endif

		id = fork();
		if (id > 0)
		{
			pids[i] = id;
		}
		else if (id == 0)
		{
			/*child process */

#ifdef WITH_SIGNALS
			signal(SIGTERM, sig_term);
			signal(SIGINT, SIG_IGN);
#endif

			child_task();

			exit(EXIT_SUCCESS);
		}
		else
		{
			/*Case there is an error */
			printf("There was an error on parent[%d]: fork() failed.\n", pid);
			for (int k = 0; k < NUM_CHILD; k++)
			{
				if (pids[k] != 0)
					kill(pids[k], SIGTERM);
			}
			exit(EXIT_FAILURE);
		}

		sleep(1);
	}

	while (wait(NULL) != -1)
	{
		n_childs++;
		printf("Finished child processes: %d\n", n_childs);
	}
	return 0;
}

int child_task()
{

	printf("Child process child[%d] was createad from parent process parent[%d].\n", getpid(), getppid());
	sleep(10);
	printf("The child process child[%d] is done.\n", getpid());

	return 0;
}