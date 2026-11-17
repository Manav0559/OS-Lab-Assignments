#include "types.h"
#include "stat.h"
#include "user.h"

#define LOOPS 40000000

int
main(void)
{
  int prio[3] = {5, 10, 15};
  int pid[3];
  int i, old;
  volatile int x;

  old = set_priority(getpid(), 1);
  printf(1, "parent pid %d: priority %d -> 1\n", getpid(), old);

  for(i = 0; i < 3; i++){
    pid[i] = fork();
    if(pid[i] < 0){
      printf(1, "fork failed\n");
      exit();
    }
    if(pid[i] == 0){
      int start = uptime();
      for(x = 0; x < LOOPS; x++)
        ;
      printf(1, "child pid %d priority %d finished (ran from tick %d to %d)\n",
             getpid(), prio[i], start, uptime());
      exit();
    }
  }

  for(i = 0; i < 3; i++){
    old = set_priority(pid[i], prio[i]);
    printf(1, "set_priority(%d, %d) returned old priority %d\n", pid[i], prio[i], old);
  }
  printf(1, "set_priority(9999, 5) = %d, set_priority(%d, 21) = %d\n",
         set_priority(9999, 5), pid[0], set_priority(pid[0], 21));

  for(i = 0; i < 3; i++)
    wait();
  printf(1, "Done\n");
  exit();
}
