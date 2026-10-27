#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  char *p;
  int i;

  printf(1, "===== [1] vmprint at start-up =====\n");
  vmprint();

  p = sbrk(8 * 4096);
  printf(1, "\n===== [2] after sbrk(8 * 4096), heap not touched yet =====\n");
  vmprint();

  for(i = 0; i < 8; i++)
    p[i * 4096] = i;
  printf(1, "\n===== [3] after touching all 8 heap pages =====\n");
  vmprint();

  sbrk(-4096);
  printf(1, "\n===== [4] after sbrk(-4096) =====\n");
  vmprint();
  exit();
}
