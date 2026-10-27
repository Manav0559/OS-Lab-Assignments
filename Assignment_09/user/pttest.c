#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  int before, mid, after, i;
  char *p;

  printf(1, "===== [1] vmprint at start-up =====\n");
  before = vmprint();

  p = sbrk(4 * 4096);
  printf(1, "\n===== [2] right after sbrk(4 * 4096) (lazy: nothing mapped yet) =====\n");
  mid = vmprint();

  for(i = 0; i < 4; i++)
    p[i * 4096] = 'a' + i;
  printf(1, "\n===== [3] after the 4 new pages are first used =====\n");
  after = vmprint();

  printf(1, "\nvalid user pages: start %d, after sbrk %d, after use %d\n", before, mid, after);
  printf(1, "new pages = %d -> %s\n", after - before, after - before == 4 ? "exactly four new pages" : "unexpected");
  exit();
}
