Name        : Manav Singla
Roll number : 2402MC02
Set number  : 1
Platform    : xv6-public (x86) on QEMU, built and run with  make qemu-nox CPUS=1

Contents
  xv6/                   complete xv6 directory after make clean (all kernel and user changes)
  2402MC02_Set1.tar.gz   tar czf 2402MC02_Set1.tar.gz xv6/
  Q1_sources/            files changed/added for Question 1
  Q2_sources/            files changed/added for Question 2
  Q1_output.png/.txt     synctest run in the xv6 shell (roll number echoed first)
  Q2_output.png/.txt     memtest run in the xv6 shell (roll number echoed first)
  xv6_changes.diff       unified diff of every change against unmodified xv6-public
  logs/usertests.txt     stock usertests on the modified kernel: ALL TESTS PASSED

System calls added: 22 sem_init, 23 sem_wait, 24 sem_post, 25 freepages, 26 va2pa.
Run in the xv6 shell:  synctest   and   memtest

Question 1 - Counting semaphores
- sem.c (new, sem.o added to OBJS): a table of 8 semaphores {value, used} protected by one
  spinlock; seminit() initialises the lock and is called from main() at boot (main.c).
- sem_init(id, value) sets value and marks the entry in use; returns -1 for id outside 0..7 or value < 0.
- sem_wait sleeps with sleep(sem, &semtable.lock) in a while (value == 0) loop, re-checking after every
  wake-up, then decrements; sem_post increments and calls wakeup() while holding the same lock, so no
  wake-up is lost. Both return -1 for a bad id or a semaphore that was never initialised.
- Plumbing: syscall.h numbers, syscall.c table, sysproc.c wrappers (argint), usys.S stubs, user.h and
  defs.h prototypes.
- synctest.c: sems 0,1,2 = 1,0,0; child i waits on sem i, prints its letter, posts sem (i+1)%3, 5 times.
  Output is exactly A B C A B C ... (15 letters) then Done; it also shows the -1 error cases.

Question 2 - Memory statistics and address translation
- kalloc.c: new kmem.nfree counter, ++ in kfree() and -- in kalloc() inside the existing kmem.lock
  section (when use_lock is set); freepages() returns it.
- vm.c: walkpgdir() is static, so new helper uva2pa(pgdir, va) (declared in defs.h) walks the page table
  with alloc=0 and returns PTE_ADDR(*pte) | (va & 0xFFF), or -1 if not present or PTE_U is clear.
- sys_va2pa() calls uva2pa(myproc()->pgdir, va), so it always translates in the calling process.
- memtest.c: freepages at start (56789) and after sbrk(10*4096) (56779, exactly 10 pages); va/pa of a
  global (0xC20 -> 0xDE9FC20); va2pa(0x7FFFFFFF) = -1; after fork the global has the same va but parent
  pa 0xDE9FC20 vs child pa 0xDEABC20 (fork copies the pages); freepages before fork 56778, after fork
  56698, after wait() 56778 again, so no leak. (The 1-page drop before fork is the pipe() the child uses
  to send its physical address back to the parent, so the two processes never print at the same time.)

Makefile note: besides adding sem.o and _synctest/_memtest, the TOOLPREFIX auto-detection also accepts
the i686-elf- toolchain (Homebrew on macOS), and -Wno-array-bounds -Wno-infinite-recursion
-Wno-stringop-overflow -Wno-unused-but-set-variable are added so unmodified xv6 compiles with newer gcc
(14+). Neither changes the kernel's behaviour.
