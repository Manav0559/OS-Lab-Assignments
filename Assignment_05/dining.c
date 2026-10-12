#include "kernel/types.h"
#include "user/user.h"

#define NPHIL 5
#define NCYCLES 5

void philosopher(int id)
{
  int left = id;
  int right = (id + 1) % NPHIL;
  int first = left < right ? left : right;
  int second = left < right ? right : left;

  for (int cycle = 0; cycle < NCYCLES; cycle++) {
    printf("Philosopher %d: THINKING\n", id);
    pause(1);

    printf("Philosopher %d: HUNGRY\n", id);
    sem_wait(first);
    sem_wait(second);

    printf("Philosopher %d: EATING (cycle %d)\n", id, cycle);
    pause(2);

    sem_signal(second);
    sem_signal(first);
    printf("Philosopher %d: THINKING -> HUNGRY -> EATING -> THINKING\n", id);
  }
}
int main(void)
{
  for (int i = 0; i < NPHIL; i++)
    sem_init(i, 1);

  for (int i = 0; i < NPHIL; i++) {
    int pid = fork();
    if (pid == 0) {
      philosopher(i);
      exit(0);
    }
  }

  for (int i = 0; i < NPHIL; i++)
    wait(0);

  printf("All philosophers finished %d cycles, no deadlock\n", NCYCLES);
  exit(0);
}
