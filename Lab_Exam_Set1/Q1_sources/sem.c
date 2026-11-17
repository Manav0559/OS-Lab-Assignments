// Counting semaphores for user processes (Question 1).
// A fixed table of NSEM semaphores lives in the kernel, so it is shared by
// a parent and every child it creates with fork().

#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

#define NSEM 8

struct sem {
  int value;      // current count (never negative)
  int used;       // 1 once sem_init has been called on this id
};

struct {
  struct spinlock lock;   // protects every entry of sem[]
  struct sem sem[NSEM];
} semtable;

// Called once from main() at boot.
void
seminit(void)
{
  initlock(&semtable.lock, "semtable");
}

static int
badid(int id)
{
  return id < 0 || id >= NSEM;
}

// Set semaphore id to value (value >= 0). Returns 0, or -1 on bad id/value.
int
sem_init(int id, int value)
{
  if(badid(id) || value < 0)
    return -1;
  acquire(&semtable.lock);
  semtable.sem[id].value = value;
  semtable.sem[id].used = 1;
  release(&semtable.lock);
  return 0;
}

// P(): block while the value is 0, then decrement it.
int
sem_wait(int id)
{
  struct sem *s;

  if(badid(id))
    return -1;
  s = &semtable.sem[id];
  acquire(&semtable.lock);
  if(!s->used){
    release(&semtable.lock);
    return -1;
  }
  // Re-check after every wake-up: sem_post wakes all waiters but only
  // one of them may get the unit. sleep() releases semtable.lock
  // atomically, so a post between the check and the sleep is not lost.
  while(s->value == 0){
    if(myproc()->killed){
      release(&semtable.lock);
      return -1;
    }
    sleep(s, &semtable.lock);
  }
  s->value--;
  release(&semtable.lock);
  return 0;
}

// V(): increment the value and wake every process sleeping on it.
int
sem_post(int id)
{
  struct sem *s;

  if(badid(id))
    return -1;
  s = &semtable.sem[id];
  acquire(&semtable.lock);
  if(!s->used){
    release(&semtable.lock);
    return -1;
  }
  s->value++;
  wakeup(s);   // called with semtable.lock held, same lock sem_wait sleeps with
  release(&semtable.lock);
  return 0;
}
