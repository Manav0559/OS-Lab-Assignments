# OS Assignment 10 — Memory Accounting, Address Translation, Page Protection, Page Replacement (xv6 x86)

Built on mit-pdos/xv6-public (x86), run with qemu-system-i386 (1 CPU).

## Layout

```
Lab10_Submission/
├── kernel/          modified files (full): kalloc.c vm.c trap.c sysproc.c syscall.c syscall.h
│                    defs.h user.h usys.S Makefile
├── user/            memtest.c, vttest.c, pagetest.c, pagereplace.c
├── diffs/           unified diffs against upstream xv6-public
├── screenshots/     terminal output for each question
├── output_logs/     console logs (+ usertests run)
└── report.pdf       2-page report
```

## Results

| Q | Change | Test | Result |
|---|--------|------|--------|
| 1 | counters in `kmem` (`nalloc`, `nfreed`, `nfree`, `npages`), updated in `kalloc()`/`kfree()`; boot-time `freerange()` not counted as frees; `memstats()` | `memtest` | sbrk 64 pages: in-use 276 → 340 (+64); sbrk(-): 340 → 276 (-64), PASS |
| 2 | `walkpgdir()` exported; `vtop()` = `PTE_ADDR(*pte) \| (va & 0xFFF)` after P and U checks | `vttest` | `0x3123 → 0x0dfbc123`; invalid: no page table (-1), PTE not present (-2), PTE_U clear (-3) |
| 3 | `pageinfo()` prints PFN + P/W/U; `setreadonly()` clears `PTE_W` + flushes TLB (user pages only); `trap.c` decodes `T_PGFLT` | `pagetest` | PTE 0x…067 → 0x…065; write faults with err 7 (present, write, user), process killed |
| 4 | user program: FIFO, LRU, Optimal | `pagereplace` | 3 frames: FIFO 15, LRU 12, OPT 9; 4: 10/8/8; 5: 9/7/7 |

## System calls

| # | Call | Returns |
|---|------|---------|
| 22 | `int memstats(void)` | pages currently allocated (prints all statistics) |
| 23 | `int vtop(int va)` | physical address, or -1/-2/-3 if invalid |
| 24 | `int pageinfo(int va)` | PTE flag bits, or -1 if unmapped (prints PFN, P/W/U) |
| 25 | `int setreadonly(int va)` | 0, or -1 if not a present user page |

## Build and run

```
git clone https://github.com/mit-pdos/xv6-public && cd xv6-public
cp ../kernel/* . && cp ../user/* .
brew install i686-elf-gcc qemu
make qemu-nox CPUS=1
$ memtest                  (optional: memtest N  for N pages, 1..512)
$ vttest
$ pagetest
$ pagereplace              default string, traces with 3 frames + 3/4/5 table
$ pagereplace 4            default string, traces with 4 frames
$ pagereplace 3 1,2,3,4,1,2,5,1,2,3,4,5    custom string (Belady's anomaly for FIFO)
```

`usertests` passes on the modified kernel (`output_logs/usertests.txt`).
