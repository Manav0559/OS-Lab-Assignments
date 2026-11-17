Name        : Manav Singla
Roll number : 2402MC02
Set number  : 2

Built on xv6-public (x86) and run with: make qemu-nox CPUS=1
Inside xv6 run: prottest and schedtest
New system calls: 22 mprotect, 23 munprotect, 24 set_priority

What is in this folder
  xv6/                  full xv6 tree after make clean
  2402MC02_Set2.tar.gz  tar czf of xv6/
  Q1_sources/           files I changed for Q1
  Q2_sources/           files I changed for Q2
  Q1_output, Q2_output  screenshot (.png) and console log (.txt) of each test, roll number echoed first
  xv6_changes.diff      everything I changed compared to plain xv6-public

Question 1 - mprotect / munprotect
- vm.c: added setwritable(pgdir, sz, addr, len, writable). walkpgdir() is static so the page walk
  has to live in vm.c; the function is declared in defs.h.
- It returns -1 if addr is not page aligned, len <= 0, addr + len*PGSIZE goes past sz, or any page
  in the range is missing or not a user page. All pages are checked first, then PTE_W is cleared or
  set on every page, then the TLB is flushed with lcr3(V2P(pgdir)).
- sysproc.c: sys_mprotect and sys_munprotect read the two arguments and call it with the
  calling process's pgdir and sz. Usual plumbing in syscall.h, syscall.c, usys.S, user.h.
- prottest.c: sbrk(4096) gives a page aligned page (0x3000), A is written and still readable after
  mprotect, a forked child that writes to it is killed by the kernel (trap 14, err 7), the parent
  still sees A, after munprotect writing C works, and the bad cases all return -1.

Question 2 - priority scheduler
- proc.h: new field priority in struct proc. proc.c: allocproc() sets it to 10, so every new process
  (fork children too) starts at 10.
- proc.c: set_priority(pid, priority) checks 1..20, looks for the pid in the process table while
  holding ptable.lock, sets the new value and returns the old one, or -1.
- proc.c scheduler(): each pass (with ptable.lock held) goes through the whole table and keeps the
  RUNNABLE process with the smallest number; on a tie the first one in the table wins. Then it
  switches to it exactly like the original code.
- schedtest.c: the parent sets itself to 1, forks three children, gives them 5, 10 and 15, then waits.
  The children run one after another in that order (ticks 97-108, 109-119, 120-131), then Done.

Makefile: added _prottest and _schedtest to UPROGS. I also let the TOOLPREFIX check find the
i686-elf- tools (macOS) and turned off four newer gcc warnings so plain xv6 builds with gcc 13/14.
Nothing about how the kernel behaves changes because of that.

Note on usertests (logs/usertests_partial.txt): every test up to preempt passes, then preempt hangs.
That is a side effect of the scheduler the question asks for. preempt forks three children that spin
forever at priority 10, the same as the parent, and on a tie the first one in the table always wins,
so if a spinning child sits before the parent in the table the parent never runs again to kill it.
Plain round-robin would rotate through them; strict priority with first-in-table ties does not.
