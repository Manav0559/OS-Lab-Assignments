#include "types.h"
#include "stat.h"
#include "user.h"

#define NPAGES 32

int
main(void)
{
  char *p;
  int pages[5] = {0, 3, 10, 20, 31};
  int i, f0, f1;

  f0 = getfaults();
  p = sbrk(NPAGES * 4096);
  printf(1, "sbrk(%d * 4096) returned 0x%x, page faults so far: %d\n", NPAGES, (uint)p, getfaults());

  for(i = 0; i < 5; i++){
    p[pages[i] * 4096 + 100] = 'x';
    printf(1, "wrote page %d (va 0x%x), faults handled = %d\n", pages[i], (uint)(p + pages[i] * 4096), getfaults());
  }
  for(i = 0; i < 5; i++)
    p[pages[i] * 4096 + 200] = 'y';
  f1 = getfaults();
  printf(1, "second write to the same 5 pages, faults handled = %d\n", f1);
  printf(1, "page faults handled for 5 pages: %d -> %s\n", f1 - f0, f1 - f0 == 5 ? "PASS" : "FAIL");

  printf(1, "\nchild accesses 0x%x (above sz = 0x%x):\n", (uint)(p + NPAGES * 4096 + 4096), (uint)(p + NPAGES * 4096));
  if(fork() == 0){
    p[NPAGES * 4096 + 4096] = 'z';
    printf(1, "ERROR: child survived an access above sz\n");
    exit();
  }
  wait();
  printf(1, "parent still running, its data intact: p[0+100] = %c, faults = %d\n", p[100], getfaults());
  exit();
}
