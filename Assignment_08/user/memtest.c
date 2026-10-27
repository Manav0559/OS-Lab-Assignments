#include "types.h"
#include "stat.h"
#include "user.h"
#include "meminfo.h"

#define GROW 64

void
show(char *label, struct meminfo *m)
{
  getmeminfo(m);
  printf(1, "%s\n    total %d  free %d  proc_pages %d  pt_pages %d\n",
         label, m->total_pages, m->free_pages, m->proc_pages, m->pt_pages);
}

int
main(void)
{
  struct meminfo start, grown, before, alive, after, shrunk;
  char *p, c;
  int fd[2], i;

  show("[1] at start", &start);

  p = sbrk(GROW * 4096);
  for(i = 0; i < GROW; i++)
    p[i * 4096] = 1;
  show("[2] after sbrk(64 pages) and touching them", &grown);
  printf(1, "    free pages used: %d, proc_pages grew by %d\n",
         start.free_pages - grown.free_pages, grown.proc_pages - start.proc_pages);

  pipe(fd);
  show("[3] before fork", &before);
  if(fork() == 0){
    read(fd[0], &c, 1);
    exit();
  }
  show("[4] child alive (copy of the parent)", &alive);
  printf(1, "    fork consumed %d free pages\n", before.free_pages - alive.free_pages);

  write(fd[1], "x", 1);
  wait();
  show("[5] after child exited and was reaped", &after);
  printf(1, "    leak check: free before fork %d, after reap %d -> %s\n", before.free_pages, after.free_pages,
         before.free_pages == after.free_pages ? "PASS (no leak)" : "FAIL (leak)");

  close(fd[0]);
  close(fd[1]);
  sbrk(-GROW * 4096);
  show("[6] after sbrk(-64 pages) and closing the pipe", &shrunk);
  printf(1, "    free pages vs start: %d -> %s\n", shrunk.free_pages - start.free_pages,
         shrunk.free_pages == start.free_pages ? "back to the original value" : "differs");
  exit();
}
