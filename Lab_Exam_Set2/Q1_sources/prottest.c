#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  char *p;
  int pid;

  p = sbrk(4096);
  if(p == (char*)-1){
    printf(1, "sbrk failed\n");
    exit();
  }
  printf(1, "page at 0x%x, page aligned: %s\n", (uint)p, ((uint)p % 4096) == 0 ? "yes" : "no");
  p[0] = 'A';
  printf(1, "wrote %c\n", p[0]);

  printf(1, "\n(i) mprotect(p, 1) = %d\n", mprotect(p, 1));
  printf(1, "    read after mprotect: %c\n", p[0]);

  printf(1, "\n(ii) child tries to write to the read-only page\n");
  pid = fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    exit();
  }
  if(pid == 0){
    p[0] = 'B';
    printf(1, "    child: write went through, this is wrong\n");
    exit();
  }
  wait();
  printf(1, "    parent: child %d is gone, page still has %c\n", pid, p[0]);

  printf(1, "\n(iii) munprotect(p, 1) = %d\n", munprotect(p, 1));
  p[0] = 'C';
  printf(1, "     write after munprotect: %c\n", p[0]);

  printf(1, "\n(iv) error cases\n");
  printf(1, "     mprotect(p + 1, 1)    = %d   unaligned address\n", mprotect(p + 1, 1));
  printf(1, "     mprotect(p, 0)        = %d   len = 0\n", mprotect(p, 0));
  printf(1, "     mprotect(p, -2)       = %d   len < 0\n", mprotect(p, -2));
  printf(1, "     mprotect(p, 2)        = %d   range goes past sz\n", mprotect(p, 2));
  printf(1, "     mprotect(p + 4096, 1) = %d   page after sz\n", mprotect(p + 4096, 1));
  exit();
}
