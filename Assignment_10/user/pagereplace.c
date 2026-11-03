#include "types.h"
#include "stat.h"
#include "user.h"

#define MAXREF 64
#define MAXFR 10
#define FIFO 0
#define LRU 1
#define OPT 2

int defref[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
char *name[] = {"FIFO", "LRU", "Optimal"};

int ref[MAXREF], len;

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

static void
percent(int x)
{
  int r = (x * 10000 + len / 2) / len;

  pad(r / 100, 3);
  printf(1, ".%s%d%%", r % 100 < 10 ? "0" : "", r % 100);
}

static int
find(int *fr, int n, int page)
{
  for(int i = 0; i < n; i++)
    if(fr[i] == page)
      return i;
  return -1;
}

static int
nextuse(int page, int t)
{
  for(int k = t + 1; k < len; k++)
    if(ref[k] == page)
      return k;
  return len;
}

static int
victim(int alg, int *fr, int *stamp, int n, int t, int *fifonext)
{
  int v = 0;

  if(alg == FIFO){
    v = *fifonext;
    *fifonext = (*fifonext + 1) % n;
    return v;
  }
  for(int k = 1; k < n; k++){
    if(alg == LRU && stamp[k] < stamp[v])
      v = k;
    if(alg == OPT && nextuse(fr[k], t) > nextuse(fr[v], t))
      v = k;
  }
  return v;
}

static int
simulate(int alg, int n, int verbose)
{
  int fr[MAXFR], stamp[MAXFR], used = 0, faults = 0, fifonext = 0;

  for(int i = 0; i < n; i++){
    fr[i] = -1;
    stamp[i] = -1;
  }
  if(verbose)
    printf(1, "%s, %d frames\n", name[alg], n);
  for(int t = 0; t < len; t++){
    int i = find(fr, n, ref[t]), evicted = -1;
    int hit = i >= 0;

    if(!hit){
      faults++;
      if(used < n)
        i = used++;
      else {
        i = victim(alg, fr, stamp, n, t, &fifonext);
        evicted = fr[i];
      }
      fr[i] = ref[t];
    }
    stamp[i] = t;
    if(verbose){
      printf(1, "  ");
      pad(t + 1, 2);
      printf(1, ". ref %d  [", ref[t]);
      for(int k = 0; k < n; k++){
        if(fr[k] < 0)
          printf(1, " -");
        else
          printf(1, " %d", fr[k]);
      }
      printf(1, " ]  %s", hit ? "hit" : "FAULT");
      if(evicted >= 0)
        printf(1, " (replaced %d)", evicted);
      printf(1, "\n");
    }
  }
  if(verbose)
    printf(1, "  => %s page faults = %d, hits = %d\n\n", name[alg], faults, len - faults);
  return faults;
}

static void
table(void)
{
  int f[3], best;

  printf(1, "Comparison (%d references)\n", len);
  printf(1, "frames | algorithm | faults | hits | hit ratio | fault ratio\n");
  printf(1, "-------+-----------+--------+------+-----------+------------\n");
  for(int n = 3; n <= 5; n++){
    best = 0;
    for(int a = 0; a < 3; a++){
      f[a] = simulate(a, n, 0);
      if(f[a] < f[best])
        best = a;
    }
    for(int a = 0; a < 3; a++){
      printf(1, "   %d   | %s", n, name[a]);
      for(int k = strlen(name[a]); k < 9; k++)
        printf(1, " ");
      printf(1, " |");
      pad(f[a], 6);
      printf(1, "  |");
      pad(len - f[a], 4);
      printf(1, "  |  ");
      percent(len - f[a]);
      printf(1, "  |  ");
      percent(f[a]);
      printf(1, "%s\n", f[a] == f[best] ? "  <- best" : "");
    }
    printf(1, "-------+-----------+--------+------+-----------+------------\n");
  }
}

int
main(int argc, char *argv[])
{
  int frames = 3, f[3];

  if(argc >= 3){
    frames = atoi(argv[1]);
    for(int i = 2; i < argc; i++){
      char *s = argv[i];
      while(*s && len < MAXREF){
        ref[len++] = atoi(s);
        while(*s && *s != ',')
          s++;
        if(*s == ',')
          s++;
      }
    }
  } else {
    if(argc == 2)
      frames = atoi(argv[1]);
    for(len = 0; len < sizeof(defref) / sizeof(defref[0]); len++)
      ref[len] = defref[len];
  }
  if(frames < 1 || frames > MAXFR || len == 0){
    printf(2, "usage: pagereplace [frames 1..%d] [ref,ref,...]\n", MAXFR);
    exit();
  }

  printf(1, "reference string (%d refs):", len);
  for(int i = 0; i < len; i++)
    printf(1, " %d", ref[i]);
  printf(1, "\n\n");

  for(int a = 0; a < 3; a++)
    f[a] = simulate(a, frames, 1);

  printf(1, "Total page faults with %d frames: FIFO = %d, LRU = %d, Optimal = %d\n\n",
         frames, f[FIFO], f[LRU], f[OPT]);
  table();
  exit();
}
