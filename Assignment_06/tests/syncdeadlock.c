#include "kernel/types.h"
#include "user/user.h"

#define PRINTER 0
#define SCANNER 1
#define DISK 2
#define NPROC 5
#define CYCLES 3

char *names[3] = {"printer", "scanner", "disk"};
int need[NPROC][2] = {
    {PRINTER, SCANNER},
    {SCANNER, DISK},
    {PRINTER, DISK},
    {PRINTER, SCANNER},
    {SCANNER, DISK},
};

void
worker(int id)
{
  int first = need[id][0];
  int second = need[id][1];

  for (int cycle = 0; cycle < CYCLES; cycle++) {
    printf("P%d requesting %s\n", id, names[first]);
    sem_wait(first);
    printf("P%d granted %s\n", id, names[first]);

    printf("P%d requesting %s\n", id, names[second]);
    sem_wait(second);
    printf("P%d granted %s\n", id, names[second]);

    printf("P%d starts work (cycle %d)\n", id, cycle);
    pause(2);

    printf("P%d releasing %s and %s\n", id, names[second], names[first]);
    sem_signal(second);
    sem_signal(first);
    pause(1);
  }
  printf("P%d finished all cycles\n", id);
}

int
main(void)
{
  sem_init(PRINTER, 2);
  sem_init(SCANNER, 1);
  sem_init(DISK, 2);

  for (int i = 0; i < NPROC; i++) {
    int pid = fork();
    if (pid == 0) {
      worker(i);
      exit(0);
    }
  }

  for (int i = 0; i < NPROC; i++)
    wait(0);

  printf("All processes completed, no deadlock\n");
  exit(0);
}
