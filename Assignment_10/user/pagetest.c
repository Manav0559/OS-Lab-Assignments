#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096

int
main(void)
{
  char *page, ok;
  int p[2], pid, r;

  page = sbrk(PGSIZE);
  page[0] = 'X';
  printf(1, "pagetest: allocated one page with sbrk(4096) at va 0x%x, wrote '%c'\n\n", page, page[0]);

  printf(1, "[1] pageinfo() of the new page:\n");
  pageinfo((int)page);
  printf(1, "    for comparison, pageinfo() of an unmapped and a kernel address:\n");
  pageinfo((int)page + 8 * PGSIZE);
  pageinfo(0x80100000);

  printf(1, "\n[2] setreadonly(0x%x) returned %d\n", page, setreadonly((int)page));
  pageinfo((int)page);
  printf(1, "    reads still work: page[0] = '%c'\n", page[0]);
  printf(1, "    setreadonly(0x80100000) on a kernel page returned %d (refused)\n",
         setreadonly(0x80100000));

  printf(1, "\n[3] child process (fork copies the read-only PTE) writes to the page:\n");
  pipe(p);
  pid = fork();
  if(pid == 0){
    close(p[0]);
    printf(1, "    child pid %d: page[0] = 'Y' ...\n", getpid());
    page[0] = 'Y';
    ok = 1;
    write(p[1], &ok, 1);
    printf(1, "    child: write succeeded (protection NOT enforced)\n");
    exit();
  }
  close(p[1]);
  r = read(p[0], &ok, 1);
  wait();
  close(p[0]);
  if(r == 0)
    printf(1, "    parent: child was killed by the page fault before finishing its write\n");
  else
    printf(1, "    parent: child's write was NOT blocked -- FAIL\n");
  printf(1, "    parent: page[0] is still '%c'\n", page[0]);

  printf(1, "\n[4] parent now writes to the read-only page itself:\n");
  page[0] = 'Z';
  printf(1, "    parent: write succeeded (protection NOT enforced) -- FAIL\n");
  exit();
}
