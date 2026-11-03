#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[];

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;

  uint npages;
  uint nfree;
  uint nalloc;
  uint nfreed;
  int seeding;
} kmem;

void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  kmem.seeding = 1;
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
  kmem.seeding = 0;
}

void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  kmem.nfree++;
  if(kmem.seeding)
    kmem.npages++;
  else
    kmem.nfreed++;
  if(kmem.use_lock)
    release(&kmem.lock);
}

char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    kmem.nfree--;
    kmem.nalloc++;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

int
memstats(void)
{
  uint npages, nfree, nalloc, nfreed;

  acquire(&kmem.lock);
  npages = kmem.npages;
  nfree = kmem.nfree;
  nalloc = kmem.nalloc;
  nfreed = kmem.nfreed;
  release(&kmem.lock);

  cprintf("  [memstats] total allocated pages : %d\n", nalloc);
  cprintf("  [memstats] total freed pages     : %d\n", nfreed);
  cprintf("  [memstats] currently allocated   : %d  (%d KB)\n", nalloc - nfreed, (nalloc - nfreed) * 4);
  cprintf("  [memstats] available free pages  : %d  (%d KB)\n", nfree, nfree * 4);
  cprintf("  [memstats] pages managed         : %d  (check: managed - free = %d)\n",
          npages, npages - nfree);
  return nalloc - nfreed;
}
