#include "kernel/types.h"
#include "user/user.h"

#define MAXBUF 32
#define EMPTY 0
#define FULL 1
#define MUTEX 2
#define NITEMS 20

struct shared {
  int buf[MAXBUF];
  int size;
  int in;
  int out;
};

int
main(int argc, char *argv[])
{
  int size = argc > 1 ? atoi(argv[1]) : 5;
  if (size > MAXBUF)
    size = MAXBUF;

  struct shared *s = (struct shared *)shm_get();
  s->size = size;
  s->in = 0;
  s->out = 0;

  sem_init(EMPTY, size);
  sem_init(FULL, 0);
  sem_init(MUTEX, 1);

  int pid = fork();
  if (pid == 0)
    s = (struct shared *)shm_get();

  if (pid == 0) {
    for (int i = 0; i < NITEMS; i++) {
      sem_wait(FULL);
      sem_wait(MUTEX);
      int item = s->buf[s->out];
      s->out = (s->out + 1) % s->size;
      sem_signal(MUTEX);
      sem_signal(EMPTY);
      printf("Consumer consumed %d\n", item);
      pause(2);
    }
    exit(0);
  } else {
    for (int i = 1; i <= NITEMS; i++) {
      sem_wait(EMPTY);
      sem_wait(MUTEX);
      s->buf[s->in] = i;
      s->in = (s->in + 1) % s->size;
      sem_signal(MUTEX);
      sem_signal(FULL);
      printf("Producer produced %d\n", i);
      pause(1);
    }
    wait(0);
    exit(0);
  }
}
