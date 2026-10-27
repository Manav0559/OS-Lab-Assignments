#include "types.h"
#include "stat.h"
#include "user.h"

#define MAXREF 64
#define MAXFR 10

int defref[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};

void
showframes(int *fr, int n)
{
  printf(1, "[");
  for(int i = 0; i < n; i++){
    if(fr[i] < 0)
      printf(1, " -");
    else
      printf(1, " %d", fr[i]);
  }
  printf(1, " ]");
}

int
find(int *fr, int n, int page)
{
  for(int i = 0; i < n; i++)
    if(fr[i] == page)
      return i;
  return -1;
}

int
fifo(int *ref, int len, int n, int verbose)
{
  int fr[MAXFR], next = 0, faults = 0;

  for(int i = 0; i < n; i++)
    fr[i] = -1;
  for(int t = 0; t < len; t++){
    int hit = find(fr, n, ref[t]) >= 0;
    if(!hit){
      fr[next] = ref[t];
      next = (next + 1) % n;
      faults++;
    }
    if(verbose){
      printf(1, "  ref %d -> ", ref[t]);
      showframes(fr, n);
      printf(1, "  %s\n", hit ? "hit" : "FAULT");
    }
  }
  return faults;
}

int
lru(int *ref, int len, int n, int verbose)
{
  int fr[MAXFR], last[MAXFR], faults = 0;

  for(int i = 0; i < n; i++){
    fr[i] = -1;
    last[i] = -1;
  }
  for(int t = 0; t < len; t++){
    int i = find(fr, n, ref[t]);
    int hit = i >= 0;
    if(!hit){
      int victim = 0;
      for(int k = 0; k < n; k++){
        if(fr[k] < 0){
          victim = k;
          break;
        }
        if(last[k] < last[victim])
          victim = k;
      }
      fr[victim] = ref[t];
      i = victim;
      faults++;
    }
    last[i] = t;
    if(verbose){
      printf(1, "  ref %d -> ", ref[t]);
      showframes(fr, n);
      printf(1, "  %s\n", hit ? "hit" : "FAULT");
    }
  }
  return faults;
}

void
ratio(int hits, int len)
{
  int r = hits * 1000 / len;
  printf(1, "%d.%d%%", r / 10, r % 10);
}

int
main(int argc, char *argv[])
{
  int ref[MAXREF], len = 0, frames = 3, f, l;

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
  if(frames < 1 || frames > MAXFR){
    printf(1, "frames must be 1..%d\n", MAXFR);
    exit();
  }

  printf(1, "reference string (%d refs):", len);
  for(int i = 0; i < len; i++)
    printf(1, " %d", ref[i]);
  printf(1, "\nframes: %d\n\n", frames);

  printf(1, "FIFO:\n");
  f = fifo(ref, len, frames, 1);
  printf(1, "FIFO page faults = %d\n\n", f);
  printf(1, "LRU:\n");
  l = lru(ref, len, frames, 1);
  printf(1, "LRU page faults = %d\n\n", l);

  printf(1, "frames | FIFO faults | FIFO hit ratio | LRU faults | LRU hit ratio\n");
  printf(1, "-------+-------------+----------------+------------+--------------\n");
  for(int n = 3; n <= 5; n++){
    f = fifo(ref, len, n, 0);
    l = lru(ref, len, n, 0);
    printf(1, "   %d   |     %s%d      |     ", n, f < 10 ? " " : "", f);
    ratio(len - f, len);
    printf(1, "      |     %s%d     |    ", l < 10 ? " " : "", l);
    ratio(len - l, len);
    printf(1, "\n");
  }
  exit();
}
