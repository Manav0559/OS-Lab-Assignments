#include "kernel/types.h"
#include "user/user.h"

#define MUTEX 0
#define WRT 1
#define NREADERS 3
#define NWRITERS 2
#define ITERS 3

struct shared {
  int shared_data;
  int read_count;
};

void
reader(struct shared *s, int id)
{
  for (int i = 0; i < ITERS; i++) {
    sem_wait(MUTEX);
    s->read_count++;
    if (s->read_count == 1)
      sem_wait(WRT);
    sem_signal(MUTEX);

    printf("Reader %d (pid %d) read shared_data = %d\n", id, getpid(),
           s->shared_data);
    pause(1);

    sem_wait(MUTEX);
    s->read_count--;
    if (s->read_count == 0)
      sem_signal(WRT);
    sem_signal(MUTEX);
    pause(1);
  }
}

void
writer(struct shared *s, int id)
{
  for (int i = 0; i < ITERS; i++) {
    sem_wait(WRT);
    s->shared_data++;
    printf("Writer %d (pid %d) wrote shared_data = %d\n", id, getpid(),
           s->shared_data);
    pause(2);
    sem_signal(WRT);
    pause(1);
  }
}

int
main(void)
{
  struct shared *s = (struct shared *)shm_get();
  s->shared_data = 0;
  s->read_count = 0;

  sem_init(MUTEX, 1);
  sem_init(WRT, 1);

  for (int i = 0; i < NREADERS; i++) {
    int pid = fork();
    if (pid == 0) {
      s = (struct shared *)shm_get();
      reader(s, i);
      exit(0);
    }
  }

  for (int i = 0; i < NWRITERS; i++) {
    int pid = fork();
    if (pid == 0) {
      s = (struct shared *)shm_get();
      writer(s, i);
      exit(0);
    }
  }

  for (int i = 0; i < NREADERS + NWRITERS; i++)
    wait(0);

  printf("Final shared_data = %d\n", s->shared_data);
  exit(0);
}
