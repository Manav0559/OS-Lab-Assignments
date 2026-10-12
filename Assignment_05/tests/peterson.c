#include "kernel/types.h"
#include "user/user.h"

struct shared {
  volatile int flag[2];
  volatile int turn;
  int counter;
};

int
main(void)
{
  struct shared *s = (struct shared *)shm_get();
  s->flag[0] = 0;
  s->flag[1] = 0;
  s->turn = 0;
  s->counter = 0;

  int pid = fork();
  if (pid == 0)
    s = (struct shared *)shm_get();

  int id = (pid == 0) ? 1 : 0;
  int other = 1 - id;

  for (int i = 0; i < 10; i++) {
    s->flag[id] = 1;
    s->turn = other;
    while (s->flag[other] && s->turn == other)
      ;

    s->counter++;
    printf("Process %d in CS, counter = %d\n", id, s->counter);

    s->flag[id] = 0;

    pause(1);
  }

  if (pid > 0) {
    wait(0);
    printf("Final counter = %d\n", s->counter);
  }
  exit(0);
}
