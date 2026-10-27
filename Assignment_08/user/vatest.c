#include "types.h"
#include "stat.h"
#include "user.h"

int g = 42;

int
show(char *name, void *va)
{
  int pa;

  printf(1, "%s:\n", name);
  pa = va2pa((int)va);
  if(pa == -1)
    printf(1, "  [user]   va 0x%x -> -1 (no user mapping)\n", (uint)va);
  else
    printf(1, "  [user]   va 0x%x -> pa 0x%x\n", (uint)va, (uint)pa);
  return pa;
}

int
main(void)
{
  int local = 7;
  char *m = malloc(100);
  int fd[2], childpa, parentpa;

  m[0] = 1;
  printf(1, "=== Part c: VA -> PA translation (pid %d) ===\n", getpid());
  show("global variable g", &g);
  show("local variable", &local);
  show("malloc() block", m);
  show("unmapped address 0x7FFFFFFF", (void*)0x7FFFFFFF);
  show("kernel address 0x80100000", (void*)0x80100000);

  printf(1, "\n=== Part d: fork, same VA in parent and child ===\n");
  pipe(fd);
  if(fork() == 0){
    g = 100;
    printf(1, "child  pid %d, &g = 0x%x, g = %d\n", getpid(), (uint)&g, g);
    childpa = show("child's g", &g);
    write(fd[1], &childpa, sizeof(childpa));
    exit();
  }
  read(fd[0], &childpa, sizeof(childpa));
  wait();
  printf(1, "parent pid %d, &g = 0x%x, g = %d\n", getpid(), (uint)&g, g);
  parentpa = show("parent's g", &g);
  printf(1, "\nsame VA 0x%x: parent pa 0x%x, child pa 0x%x -> %s\n", (uint)&g, (uint)parentpa, (uint)childpa,
         parentpa != childpa ? "DIFFERENT physical frames (fork copied the page)" : "same frame");
  exit();
}
