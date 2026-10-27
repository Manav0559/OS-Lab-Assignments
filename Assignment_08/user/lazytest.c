#include "types.h"
#include "stat.h"
#include "user.h"
#include "meminfo.h"

#define MB (1024 * 1024)
#define REGION (32 * MB)

void
show(char *label, struct meminfo *m)
{
  getmeminfo(m);
  printf(1, "%s\n    free %d  proc_pages %d  pt_pages %d\n", label, m->free_pages, m->proc_pages, m->pt_pages);
}

int
main(void)
{
  struct meminfo a, b, c, d, e, f;
  char *p, *q, *r, buf[8];
  int fd[2], n, local;
  int offs[4] = {0, 8 * MB, 16 * MB, REGION - 1};

  show("[1] before sbrk", &a);
  p = sbrk(REGION);
  show("[2] after sbrk(32 MB) - returns immediately", &b);
  printf(1, "    sz grew by %d pages, free pages used: %d\n", b.proc_pages - a.proc_pages, a.free_pages - b.free_pages);

  for(n = 0; n < 4; n++)
    p[offs[n]] = 'A' + n;
  show("[3] after touching 4 pages (0, 8MB, 16MB, 32MB-1)", &c);
  printf(1, "    free pages used: %d = 4 data frames + %d new page tables\n",
         b.free_pages - c.free_pages, c.pt_pages - b.pt_pages);

  printf(1, "\n[4] system calls on untouched lazy pages\n");
  pipe(fd);
  q = p + 20 * MB;
  write(fd[1], "lazy-ok", 8);
  n = read(fd[0], q, 8);
  printf(1, "    read() into untouched page 0x%x: %d bytes, \"%s\"\n", (uint)q, n, q);
  r = p + 24 * MB;
  n = write(fd[1], r, 4);
  read(fd[0], buf, 4);
  printf(1, "    write() from untouched page 0x%x: %d bytes, first byte %d (zero-filled)\n", (uint)r, n, buf[0]);
  show("    meminfo after the two system calls", &d);

  printf(1, "\n[5] fork with a sparse lazy heap\n");
  if(fork() == 0){
    p[28 * MB] = 'Z';
    printf(1, "    child: p[8MB] = %c (copied), touched p[28MB] in child only\n", p[8 * MB]);
    exit();
  }
  wait();
  show("    parent after child exit", &e);
  printf(1, "    free pages vs before fork: %d -> %s\n", e.free_pages - d.free_pages,
         e.free_pages == d.free_pages ? "no leak" : "leak");

  printf(1, "\n[6] invalid accesses (each in its own child)\n");
  if(fork() == 0){
    *(p + REGION + 10 * 4096) = 1;
    printf(1, "    ERROR: access beyond sz succeeded\n");
    exit();
  }
  wait();
  if(fork() == 0){
    char *guard = (char*)(((uint)&local & ~0xFFF) - 4096);
    *guard = 1;
    printf(1, "    ERROR: write to guard page succeeded\n");
    exit();
  }
  wait();
  printf(1, "    parent still running: only the offending children were killed\n");

  close(fd[0]);
  close(fd[1]);
  sbrk(-REGION);
  show("\n[7] after sbrk(-32 MB)", &f);
  printf(1, "    free pages vs start: %d (%d page-table pages stay allocated until exit)\n",
         f.free_pages - a.free_pages, f.pt_pages - a.pt_pages);
  exit();
}
