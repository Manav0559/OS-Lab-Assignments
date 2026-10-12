#include "kernel/types.h"
#include "user/user.h"

#define LOCK1 10
#define LOCK2 11

int
main(void)
{
  sem_init(LOCK1, 1);
  sem_init(LOCK2, 1);

  int pid = fork();
  if (pid == 0) {
    printf("Process B: trying Lock1\n");
    sem_wait(LOCK1);
    printf("Process B: acquired Lock1\n");
    pause(10);
    printf("Process B: trying Lock2\n");
    sem_wait(LOCK2);
    printf("Process B: acquired Lock2\n");
    printf("Process B: done\n");
    sem_signal(LOCK2);
    sem_signal(LOCK1);
    exit(0);
  } else {
    printf("Process A: trying Lock1\n");
    sem_wait(LOCK1);
    printf("Process A: acquired Lock1\n");
    pause(10);
    printf("Process A: trying Lock2\n");
    sem_wait(LOCK2);
    printf("Process A: acquired Lock2\n");
    printf("Process A: done\n");
    sem_signal(LOCK2);
    sem_signal(LOCK1);
    wait(0);
    printf("Both processes completed\n");
  }
  exit(0);
}
