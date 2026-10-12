#include "kernel/types.h"
#include "user/user.h"

#define N 5
#define M 3

int Allocation[N][M] = {
    {0, 1, 0},
    {2, 0, 0},
    {3, 0, 2},
    {2, 1, 1},
    {0, 0, 2},
};
int Max[N][M] = {
    {7, 5, 3},
    {3, 2, 2},
    {9, 0, 2},
    {2, 2, 2},
    {4, 3, 3},
};
int Available[M] = {3, 3, 2};
int Need[N][M];

void
computeNeed(void)
{
  for (int i = 0; i < N; i++)
    for (int j = 0; j < M; j++)
      Need[i][j] = Max[i][j] - Allocation[i][j];
}

int
isSafe(int seq[N])
{
  int work[M];
  int finish[N];
  for (int j = 0; j < M; j++)
    work[j] = Available[j];
  for (int i = 0; i < N; i++)
    finish[i] = 0;

  int count = 0;
  while (count < N) {
    int found = 0;
    for (int i = 0; i < N; i++) {
      if (finish[i])
        continue;
      int ok = 1;
      for (int j = 0; j < M; j++)
        if (Need[i][j] > work[j]) {
          ok = 0;
          break;
        }
      if (ok) {
        for (int j = 0; j < M; j++)
          work[j] += Allocation[i][j];
        finish[i] = 1;
        seq[count++] = i;
        found = 1;
      }
    }
    if (!found)
      break;
  }
  return count == N;
}

void
printSafeSequence(void)
{
  int seq[N];
  if (isSafe(seq)) {
    printf("Safe sequence: ");
    for (int i = 0; i < N; i++)
      printf("P%d ", seq[i]);
    printf("\n");
  } else {
    printf("System is NOT in a safe state\n");
  }
}

int
requestResources(int pid, int req[M])
{
  for (int j = 0; j < M; j++) {
    if (req[j] > Need[pid][j]) {
      printf("Request exceeds Need[%d] -- denied\n", pid);
      return 0;
    }
    if (req[j] > Available[j]) {
      printf("Request exceeds Available -- denied\n");
      return 0;
    }
  }

  for (int j = 0; j < M; j++) {
    Available[j] -= req[j];
    Allocation[pid][j] += req[j];
    Need[pid][j] -= req[j];
  }

  int seq[N];
  if (isSafe(seq)) {
    printf("Request granted. New safe sequence: ");
    for (int i = 0; i < N; i++)
      printf("P%d ", seq[i]);
    printf("\n");
    return 1;
  }

  for (int j = 0; j < M; j++) {
    Available[j] += req[j];
    Allocation[pid][j] -= req[j];
    Need[pid][j] += req[j];
  }
  printf("Request denied - would lead to unsafe state\n");
  return 0;
}

int
main(void)
{
  computeNeed();
  printf("Initial state:\n");
  printSafeSequence();

  int req1[M] = {1, 0, 2};
  printf("\nP1 requests (1,0,2):\n");
  requestResources(1, req1);

  int req2[M] = {0, 2, 0};
  printf("\nP4 requests (0,2,0):\n");
  requestResources(4, req2);

  exit(0);
}
