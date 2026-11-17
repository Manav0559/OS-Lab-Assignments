// Question 1(c): three children print A, B, C five times each, forced into
// the order A B C A B C ... using only semaphores (no sleep(), no spinning).
//
// Semaphore i lets child i print once. It is initialised to 1 for child A and
// 0 for B and C. After printing, child i posts semaphore (i+1)%3, handing the
// turn to the next child, so exactly one child can print at any moment.

#include "types.h"
#include "stat.h"
#include "user.h"

#define NCHILD 3
#define ROUNDS 5

int
main(void)
{
  int i, r;

  // error cases required by 1(b)
  printf(1, "sem_init(8, 1)  = %d  (invalid id)\n", sem_init(8, 1));
  printf(1, "sem_init(0, -1) = %d  (negative value)\n", sem_init(0, -1));
  printf(1, "sem_wait(-1)    = %d  (invalid id)\n", sem_wait(-1));
  printf(1, "sem_post(7)     = %d  (never initialised)\n", sem_post(7));
  printf(1, "sem_wait(7)     = %d  (never initialised)\n", sem_wait(7));

  if(sem_init(0, 1) < 0 || sem_init(1, 0) < 0 || sem_init(2, 0) < 0){
    printf(1, "synctest: sem_init failed\n");
    exit();
  }
  printf(1, "sem 0,1,2 initialised to 1,0,0\n");

  for(i = 0; i < NCHILD; i++){
    int pid = fork();
    if(pid < 0){
      printf(1, "synctest: fork failed\n");
      exit();
    }
    if(pid == 0){
      for(r = 0; r < ROUNDS; r++){
        sem_wait(i);                    // wait for my turn
        printf(1, "%c ", 'A' + i);
        sem_post((i + 1) % NCHILD);     // pass the turn on
      }
      exit();
    }
  }

  for(i = 0; i < NCHILD; i++)
    wait();
  printf(1, "\nDone\n");
  exit();
}
