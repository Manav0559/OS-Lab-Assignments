# OS Lab Assignment 8 — Memory Management in xv6 (x86)

Built on mit-pdos/xv6-public (x86, two-level paging), run with qemu-system-i386.

## Layout

```
Lab8_Submission/
├── kernel/          modified kernel files (full) + new meminfo.h
├── user/            vatest.c, memtest.c, vmprinttest.c, lazytest.c
├── diffs/           unified diffs against upstream xv6-public
├── screenshots/     terminal output per question + free-page chart
├── output_logs/     raw console logs
└── report.pdf       consolidated report
```

## Questions

| Q | Kernel changes | Test | Result |
|---|----------------|------|--------|
| 1 | `walkpgdir` exported, `va2pa()` prints PDX/PTX/offset/P-W-U | `vatest` | global, local, heap translated; 0x7FFFFFFF and kernel VA return -1; same VA 0xBEC maps to 0xDEE8BEC (parent) and 0xDF2DBEC (child) |
| 2 | free/total page counters in `kalloc.c` under `kmem.lock`, `getmeminfo()` | `memtest` | 64-page growth uses 64 frames; fork uses 134; free pages after reap equal before fork (no leak) |
| 3 | `vmprint()` two-level dump, guard page flagged, sz/PGSIZE check | `vmprinttest` | guard page at 0x1000; heap PTEs appear after touch, disappear after sbrk(-4096); check MATCH |
| 4 | lazy `sys_sbrk`, `T_PGFLT` handler, `proc->heapstart`, `copyuvm` skips absent pages | `lazytest` | 32 MB sbrk uses 0 frames; 4 touches use 4 frames + 3 page tables; read/write on lazy pages work; invalid accesses kill only the child |

## System calls

| # | Call |
|---|------|
| 22 | `int va2pa(int va)` |
| 23 | `int getmeminfo(struct meminfo *mi)` |
| 24 | `int vmprint(void)` |

## Build and run

```
git clone https://github.com/mit-pdos/xv6-public && cd xv6-public
cp ../kernel/* . && cp ../user/* .
patch -p1 < ../diffs/Makefile.diff
brew install i686-elf-gcc qemu
make qemu-nox CPUS=1
$ vatest
$ memtest
$ vmprinttest
$ lazytest
```

`usertests` passes on the modified kernel.
