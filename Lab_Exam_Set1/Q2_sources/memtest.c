// Question 2(c): free physical pages and virtual-to-physical translation.

#include "types.h"
#include "stat.h"
#include "user.h"

int gvar = 42;     // global variable in the data segment

int
main(void)
{
  int start, after_sbrk, before_fork, after_fork, after_wait;
  int pa_parent, pa_child, pid;
  int fd[2];

  // (i) at start-up
  start = freepages();
  printf(1, "(i)   freepages() at start-up        = %d\n", start);

  // (ii) after growing the heap by 10 pages
  if(sbrk(10 * 4096) == (char*)-1){
    printf(1, "memtest: sbrk failed\n");
    exit();
  }
  after_sbrk = freepages();
  printf(1, "(ii)  freepages() after sbrk(10*4096) = %d  (used %d pages)\n",
         after_sbrk, start - after_sbrk);

  // (iii) virtual and physical address of a global variable
  printf(1, "(iii) &gvar: va = 0x%x  pa = 0x%x\n", (uint)&gvar, va2pa((int)&gvar));

  // (iv) an address that is not mapped
  printf(1, "(iv)  va2pa(0x7FFFFFFF) = %d\n", va2pa(0x7FFFFFFF));

  // (v) fork: same virtual address, different physical pages
  pipe(fd);                       // child sends its pa to the parent
  before_fork = freepages();
  pid = fork();
  if(pid < 0){
    printf(1, "memtest: fork failed\n");
    exit();
  }
  if(pid == 0){
    pa_child = va2pa((int)&gvar);
    write(fd[1], &pa_child, sizeof(pa_child));
    exit();
  }
  after_fork = freepages();
  pa_parent = va2pa((int)&gvar);
  read(fd[0], &pa_child, sizeof(pa_child));
  wait();
  after_wait = freepages();
  close(fd[0]);
  close(fd[1]);

  printf(1, "(v)   after fork(): &gvar va = 0x%x in both processes\n", (uint)&gvar);
  printf(1, "      parent pa = 0x%x\n", pa_parent);
  printf(1, "      child  pa = 0x%x  -> %s\n", pa_child,
         pa_parent != pa_child ? "different (fork copied the page)" : "SAME (unexpected)");
  printf(1, "      freepages() before fork        = %d\n", before_fork);
  printf(1, "      freepages() after fork         = %d  (child holds %d pages)\n",
         after_fork, before_fork - after_fork);
  printf(1, "      freepages() after child reaped = %d  -> %s\n", after_wait,
         after_wait == before_fork ? "equal to before fork, no leak" : "LEAK");
  exit();
}
