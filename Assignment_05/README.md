Lab 5 Submission — Manav Singla, 2402MC02

Built on xv6-riscv (github.com/mit-pdos/xv6-riscv), not xv6-public, so paths/tools
below are riscv64-elf-gcc + qemu-system-riscv64.

Contents:
- diffs/        per-file patches vs pristine xv6-riscv, shared by all four questions
- peterson.c / prodcons.c / readwrite.c / dining.c   the four user programs
- tests/        same four files, duplicated per submission convention
- screenshots/  terminal output proving correct behaviour for each question
- output_logs/  full untrimmed qemu transcripts (dining.c's is truncated in its
                 screenshot for space, the complete run is here)

Shared-memory design (used by all four questions)
--------------------------------------------------
xv6 processes don't share memory after fork() (uvmcopy() gives the child its
own copied pages). Instead of intercepting fork, this submission adds one new
syscall, shm_get(), that lazily kalloc()s a single global physical page the
first time any process calls it, then mappages() that same physical page into
whichever process's page table calls shm_get() next, at a fixed virtual
address (SHMVA, one page below TRAPFRAME). Any process that wants access to
the shared region — parent or child — just calls shm_get() once itself; since
the physical frame is a kernel-global, every caller ends up pointing at the
same memory. proc_freepagetable() was given one extra uvmunmap() (do_free=0)
so a process that mapped this page doesn't panic the pagetable walk on exit,
without ever freeing the shared frame itself.

This single mechanism is reused as-is for Q1 (Peterson's flag[]/turn), Q2
(the circular buffer), and Q3 (shared_data + read_count) — no per-question
kernel changes were needed beyond the one shared syscall.

Semaphore design (used by Q2, Q3, Q4)
--------------------------------------
Three more syscalls (sem_init, sem_wait, sem_signal) implement a minimal
counting semaphore, indexed 0..15, backed directly by xv6's own
sleep_prepare()/sleep()/wakeup() (the same pattern sys_pause() already uses):
sem_wait spins down inside a spinlock, calling sleep_prepare(&sem) then
releasing the lock and calling sleep() when the count is <= 0; sem_signal
increments the count and calls wakeup(&sem). Each question calls sem_init()
with its own small set of indices at the start of its own program, so the
same kernel table is freely reused run to run.

Question 1 — Peterson's Algorithm (peterson.c)
------------------------------------------------
Parent calls shm_get(), zeroes flag[2]/turn/counter, then forks. The child
also calls shm_get() (mapping the same physical page into its own address
space) before entering the loop. flag/turn are declared volatile so the
busy-wait actually re-reads memory instead of caching a stale register value.
Each process alternates CS entry with a pause(1) "remainder section" (rather
than a spin-delay) so the scheduler actually interleaves the two processes —
run with CPUS=1 output shows clean 0,1,0,1,... alternation and a final
counter of exactly 20 (10 increments x 2 processes, no lost updates).

Question 2 — Producer-Consumer (prodcons.c)
----------------------------------------------
Circular buffer (configurable size, default 5) lives in the shm page.
EMPTY/FULL/MUTEX semaphores are the classic bounded-buffer trio. Producer
(parent) makes items 1..20, sleeping 1 tick between each; consumer (child)
removes and prints them, sleeping 2 ticks between each — the slower consumer
means the buffer fills and the producer visibly blocks on sem_wait(EMPTY)
until the consumer catches up. Output confirms strict in-order delivery
with no item lost or duplicated.

Question 3 — Readers-Writers (readwrite.c)
---------------------------------------------
First readers-writers (readers-priority): read_count is itself protected by
MUTEX, and only the first reader to arrive takes WRT (blocking writers);
only the last reader to leave releases it. 3 readers + 2 writers are forked
from a single parent, each running 3 iterations. Output shows all three
readers observing the same shared_data value back-to-back (concurrent read),
and each writer's increment appearing in its own exclusive block with no
reader value visible mid-write.

Question 4 — Dining Philosophers (dining.c)
-----------------------------------------------
5 philosophers, 5 forks, each fork a binary semaphore (sem_init(i, 1)).
Deadlock-avoidance strategy: resource ordering — every philosopher picks up
its lower-numbered fork first and higher-numbered fork second (first =
min(left, right), second = max(left, right)), regardless of which side that
fork happens to be on. This breaks the circular-wait condition that causes
the classic deadlock (all five picking up their left fork simultaneously):
since fork 4 is always acquired after fork 0..3 by whoever needs it, there
can never be a cycle of philosophers each holding one fork and waiting on
the next. All 5 philosophers complete 5 THINKING->HUNGRY->EATING->THINKING
cycles with no hang and no philosopher going unfed for the run.

Rebuild
-------
  1. clone a fresh pristine xv6-riscv
  2. apply the diffs from the repo root:
       patch -p1 < diffs/kernel_memlayout.h.diff
       patch -p1 < diffs/kernel_syscall.h.diff
       patch -p1 < diffs/kernel_syscall.c.diff
       patch -p1 < diffs/kernel_sysproc.c.diff
       patch -p1 < diffs/kernel_proc.c.diff
       patch -p1 < diffs/user_user.h.diff
       patch -p1 < diffs/user_usys.pl.diff
       patch -p1 < diffs/Makefile.diff
  3. copy peterson.c, prodcons.c, readwrite.c, dining.c into user/
  4. make clean && make fs.img && make qemu CPUS=1
     (single vCPU so scheduling/interleaving is cleanly observable)
  5. run peterson / prodcons / readwrite / dining at the shell prompt
