# OS Assignment 9 — Page Tables, Lazy Allocation, Page Replacement (xv6 x86)

Built on mit-pdos/xv6-public (x86), run with qemu-system-i386.

## Layout

```
Lab9_Submission/
├── kernel/          modified kernel files (full)
├── user/            pttest.c, faulttest.c, pagerepl.c, user.h
├── diffs/           unified diffs against upstream xv6-public
├── screenshots/     terminal output for each question
├── output_logs/     raw console logs
└── report.pdf       1-page report
```

## Results

| Q | Change | Test | Result |
|---|--------|------|--------|
| 1 | `vmprint()` walks user page directory via `walkpgdir()`, prints VA/PA/P-W-U, totals | `pttest` | 3 pages at start, 7 after sbrk(4*4096) + first use: exactly 4 new |
| 2 | lazy `sys_sbrk`, `T_PGFLT` handler, `copyuvm` skips absent pages, `proc->nfaults`, `getfaults()` | `faulttest` | 5 pages written -> 5 faults (PASS); access above sz kills only the child |
| 3 | user program, FIFO + LRU | `pagerepl` | 3 frames: FIFO 15, LRU 12; table for 3/4/5 frames: FIFO 15/10/9, LRU 12/8/7 |

## System calls

| # | Call |
|---|------|
| 22 | `int vmprint(void)` returns the number of valid user pages |
| 23 | `int getfaults(void)` returns page faults handled for the calling process |

## Build and run

```
git clone https://github.com/mit-pdos/xv6-public && cd xv6-public
cp ../kernel/* . && cp ../user/* .
patch -p1 < ../diffs/Makefile.diff
brew install i686-elf-gcc qemu
make qemu-nox CPUS=1
$ pttest
$ faulttest
$ pagerepl                                   default string, 3 frames + 3/4/5 table
$ pagerepl 4                                 default string, 4 frames
$ pagerepl 3 1,2,3,4,1,2,5,1,2,3,4,5         custom string (comma separated)
```

`usertests` passes on the modified kernel.
