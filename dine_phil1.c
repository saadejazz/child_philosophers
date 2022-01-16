#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

#define N 5

struct shm {
  int state[N];
};

int main()
{
  key_t key = ftok("aaaaa", 1);
  int shmid = shmget(key, sizeof(shm), IPC_CREAT | 0777);
  if (shmid == -1) {
      perror("Could not get shared memory");
      return EXIT_FAILURE;
  }

  my_pair *numbers;
  void* mem = (my_pair*) shmat(shmid, NULL, 0);
  if (mem == reinterpret_cast<void*>(-1)) {
      perror("Could not get shared memory location");
      return EXIT_FAILURE;
  } else {
      numbers = reinterpret_cast<my_pair*>(mem);
      cout << numbers->a;
  }

  return EXIT_SUCCESS;