# OS Lab Assignment 7 — xv6-riscv

Threads (clone/join), counting semaphores, deadlock detection with Banker's avoidance, and a priority-preemptive scheduler with aging.

## Layout

```
Lab7_Submission/
├── kernel/          modified kernel files (full)
├── user/            test programs + user.h
├── diffs/           unified diffs against upstream xv6-riscv
├── screenshots/     terminal output per question + Q4 graph
├── output_logs/     raw console logs
└── report.pdf       consolidated report
```

## Questions

| Q | Kernel changes | Test program | Result |
|---|----------------|--------------|--------|
| 1 | `clone`, `join`, `isthread`/`trapva` in `struct proc`, per-thread trapframe VA via `sscratch` in `trampoline.S`, thread-aware `exit`/`wait`/`fork`/`exec`/`sbrk` | `threadtest.c` | 102005 / 300000 (race) |
| 2 | `struct semaphore` table of 10, `sem_init`/`sem_wait`/`sem_signal` built on `sleep`/`wakeup` | `synctest.c`, `prodcons.c` | 104107 → 300000 exact; buffer never exceeds 5 |
| 3 | none (user space) | `deadlock.c` | deadlock / safe / safe→unsafe / real pipe deadlock all correct |
| 4 | `priority`, `waitticks`, `set_priority`, `get_priority`, priority scheduler with round-robin ties, `agetick()` | `priotest.c` | finishes in priority order; prio 20 ages to 6 and completes |

## System calls

| # | Call |
|---|------|
| 23 | `int clone(void (*fcn)(void *), void *arg, void *stack)` |
| 24 | `int join(void)` |
| 25 | `int sem_init(int id, int value)` |
| 26 | `int sem_wait(int id)` |
| 27 | `int sem_signal(int id)` |
| 28 | `int set_priority(int pid, int priority)` |
| 29 | `int get_priority(int pid)` |

## Build and run

```
cp kernel/* <xv6-riscv>/kernel/
cp user/* <xv6-riscv>/user/
cd <xv6-riscv> && patch -p1 < diffs/Makefile.diff && patch -p1 < diffs/user_usys.pl.diff
make qemu CPUS=3     threadtest, synctest, prodcons, deadlock
make qemu CPUS=1     priotest
```

`usertests -q` passes on the modified kernel.
