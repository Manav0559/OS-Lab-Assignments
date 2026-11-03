#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096

int global_var = 42;

static void
hexn(uint x, int n)
{
  char buf[12];
  int i;

  buf[0] = '0';
  buf[1] = 'x';
  for(i = n + 1; i >= 2; i--){
    buf[i] = "0123456789abcdef"[x & 0xf];
    x >>= 4;
  }
  buf[n + 2] = 0;
  printf(1, "%s", buf);
}

static void
pad(int n, int width)
{
  int digits = 1, t = n;

  while(t >= 10){
    t /= 10;
    digits++;
  }
  while(digits++ < width)
    printf(1, " ");
  printf(1, "%d", n);
}

static char *
reason(int r)
{
  if(r == -1)
    return "INVALID: no page table (PDE not present)";
  if(r == -2)
    return "INVALID: PTE not present (page unmapped)";
  if(r == -3)
    return "INVALID: PTE_U clear (kernel/guard page)";
  return "?";
}

static int
show(uint va, char *what)
{
  int pa = vtop((int)va);

  printf(1, "  ");
  hexn(va, 8);
  printf(1, "  ");
  pad((va >> 22) & 0x3ff, 4);
  printf(1, " ");
  pad((va >> 12) & 0x3ff, 4);
  printf(1, "  ");
  hexn(va & 0xfff, 3);
  printf(1, "  ");
  if(pa < 0)
    printf(1, "%s", reason(pa));
  else {
    hexn((uint)pa, 8);
    printf(1, "  %s", what);
  }
  if(pa < 0)
    printf(1, "  <- %s", what);
  printf(1, "\n");
  return pa;
}

static void
header(void)
{
  printf(1, "  virt. addr   PDX  PTX  offs.  physical addr / result\n");
  printf(1, "  ----------  ----  ---  -----  ----------------------------------------\n");
}

int
main(void)
{
  int local_var = 7;
  char *base;
  uint top, stackpg;
  int pa0, pa1, i;

  printf(1, "vttest: process size before sbrk = 0x%x\n", sbrk(0));
  base = sbrk(3 * PGSIZE);
  for(i = 0; i < 3; i++)
    base[i * PGSIZE] = 'A' + i;
  top = (uint)sbrk(0);
  printf(1, "vttest: sbrk(3*4096) returned 0x%x, new size 0x%x\n\n", base, top);

  printf(1, "valid translations (pa = frame address from PTE + page offset):\n");
  header();
  pa0 = show((uint)base, "heap page 0, offset 0");
  pa1 = show((uint)base + 0x123, "heap page 0, offset 0x123");
  show((uint)base + PGSIZE + 0x456, "heap page 1");
  show((uint)base + 2 * PGSIZE + 0xfff, "heap page 2, last byte");
  show((uint)main, "code: main()");
  show((uint)&global_var, "data: global_var");
  show((uint)&local_var, "stack: local_var");
  printf(1, "\n  check: pa(base+0x123) - pa(base) = 0x%x  (%s)\n\n",
         pa1 - pa0, pa1 - pa0 == 0x123 ? "same frame, offset preserved" : "MISMATCH");

  stackpg = (uint)&local_var & ~(PGSIZE - 1);
  printf(1, "invalid / unmapped translations:\n");
  header();
  show(top + 5 * PGSIZE, "5 pages above process size");
  show(0x40000000, "1 GB, nothing mapped there");
  show(stackpg - PGSIZE, "stack guard page");
  show(0x80100000, "kernel text (KERNBASE+1MB)");

  sbrk(-PGSIZE);
  printf(1, "\nafter sbrk(-4096) released heap page 2:\n");
  header();
  show((uint)base + 2 * PGSIZE, "heap page 2 after sbrk(-4096)");
  exit();
}
