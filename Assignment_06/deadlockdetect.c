#include "kernel/types.h"
#include "user/user.h"

#define NPROC 5
#define NRES 5
#define NONE (-1)

int alloc[NRES];
int request[NPROC];
int waitfor[NPROC][NPROC];

void
buildGraph(void)
{
  for (int i = 0; i < NPROC; i++)
    for (int j = 0; j < NPROC; j++)
      waitfor[i][j] = 0;

  for (int p = 0; p < NPROC; p++) {
    if (request[p] == NONE)
      continue;
    int r = request[p];
    int holder = alloc[r];
    if (holder != NONE && holder != p)
      waitfor[p][holder] = 1;
  }
}

void
printGraph(void)
{
  printf("Wait-for edges:\n");
  for (int i = 0; i < NPROC; i++)
    for (int j = 0; j < NPROC; j++)
      if (waitfor[i][j])
        printf("  P%d -> P%d\n", i, j);
}

int color[NPROC];
int path[NPROC + 1];
int pathlen;

int
dfs(int u)
{
  color[u] = 1;
  path[pathlen++] = u;
  for (int v = 0; v < NPROC; v++) {
    if (!waitfor[u][v])
      continue;
    if (color[v] == 1) {
      path[pathlen++] = v;
      return 1;
    }
    if (color[v] == 0 && dfs(v))
      return 1;
  }
  color[u] = 2;
  pathlen--;
  return 0;
}

void
detectDeadlock(void)
{
  for (int i = 0; i < NPROC; i++)
    color[i] = 0;

  for (int i = 0; i < NPROC; i++) {
    if (color[i] != 0)
      continue;
    pathlen = 0;
    if (dfs(i)) {
      int start = path[pathlen - 1];
      int s = 0;
      while (path[s] != start)
        s++;
      printf("Deadlock detected: ");
      for (int k = s; k < pathlen; k++) {
        printf("P%d", path[k]);
        if (k < pathlen - 1)
          printf(" -> ");
      }
      printf("\n");
      return;
    }
  }
  printf("No deadlock detected\n");
}

void
scenarioNoDeadlock(void)
{
  printf("--- Scenario 1: acyclic wait-for graph ---\n");
  for (int i = 0; i < NRES; i++)
    alloc[i] = i;
  for (int i = 0; i < NPROC; i++)
    request[i] = NONE;
  request[0] = 1;
  request[1] = 2;

  buildGraph();
  printGraph();
  detectDeadlock();
}

void
scenarioDeadlock(void)
{
  printf("\n--- Scenario 2: circular wait among 3 processes ---\n");
  for (int i = 0; i < NRES; i++)
    alloc[i] = i;
  for (int i = 0; i < NPROC; i++)
    request[i] = NONE;
  request[0] = 1;
  request[1] = 2;
  request[2] = 0;
  request[3] = 4;

  buildGraph();
  printGraph();
  detectDeadlock();
}

int
main(void)
{
  scenarioNoDeadlock();
  scenarioDeadlock();
  exit(0);
}
