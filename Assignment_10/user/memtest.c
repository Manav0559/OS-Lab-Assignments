#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096

int
main(int argc, char *argv[])
{
  int npages = 64, before, during, after, i;
  char *mem;

  if(argc > 1)
    npages = atoi(argv[1]);
  if(npages < 1 || npages > 512){
    printf(2, "usage: memtest [pages 1..512]\n");
    exit();
  }

  printf(1, "memtest: step 1 - statistics before allocation\n");
  before = memstats();

  printf(1, "\nmemtest: step 2 - sbrk(%d) to allocate %d pages\n", npages * PGSIZE, npages);
  mem = sbrk(npages * PGSIZE);
  if(mem == (char*)-1){
    printf(2, "memtest: sbrk failed\n");
    exit();
  }
  for(i = 0; i < npages; i++)
    mem[i * PGSIZE] = (char)i;
  printf(1, "memtest: heap grew 0x%x -> 0x%x, wrote 1 byte in each of the %d pages\n",
         mem, sbrk(0), npages);
  during = memstats();
  printf(1, "memtest: currently allocated changed by %d pages (expected +%d)\n",
         during - before, npages);

  printf(1, "\nmemtest: step 3 - sbrk(-%d) to release the pages\n", npages * PGSIZE);
  sbrk(-npages * PGSIZE);
  after = memstats();
  printf(1, "memtest: currently allocated changed by %d pages (expected -%d)\n",
         after - during, npages);

  if(during - before == npages && after == before)
    printf(1, "\nmemtest: PASS - all %d pages were allocated and returned to the free list\n", npages);
  else
    printf(1, "\nmemtest: FAIL\n");
  exit();
}
