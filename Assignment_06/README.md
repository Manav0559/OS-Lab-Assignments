Lab 6 Submission — Manav Singla, 2402MC02

Built on xv6-riscv (github.com/mit-pdos/xv6-riscv), continuing directly on top of
Assignment 5's kernel (shm_get / sem_init / sem_wait / sem_signal — see that
submission's README for how those work). Q1 and Q2 are pure user-space
algorithms and need no kernel changes at all; Q3 and Q4 reuse Assignment 5's
semaphore syscalls as real locks between real forked xv6 processes.

Contents:
- diffs/Makefile.diff   the only kernel-tree change this assignment needs on
                         top of Assignment 5: five new UPROGS entries, plus
                         three explicit link rules (see note below)
- bankers.c / deadlockdetect.c / resourceorder_bad.c / resourceorder_fixed.c /
  syncdeadlock.c          the five programs, also duplicated in tests/
- screenshots/           terminal output for each question (Q3's two runs,
                          Q4's is trimmed for space)
- output_logs/           full untrimmed transcripts for all five programs

A note on binary names
------------------------
xv6's filesystem has a 14-character name limit (DIRSIZ). `deadlockdetect`,
`resourceorder_bad` and `resourceorder_fixed` don't fit once the mkfs `_`
prefix is added, so the Makefile links them under shorter names instead:
deadlockdetect.c -> ddetect, resourceorder_bad.c -> rord_bad,
resourceorder_fixed.c -> rord_fix (bankers.c and syncdeadlock.c fit as-is).
The source files keep the exact names the assignment asks for; only the
compiled shell command differs. Run them as `ddetect`, `rord_bad`,
`rord_fix`, `bankers`, `syncdeadlock` at the xv6 prompt.

Question 1 — Banker's Algorithm (bankers.c)
-----------------------------------------------
Classic textbook scenario: 5 processes, 3 resource types, Available = (3,3,2),
the standard Allocation/Max matrices (safe sequence P1,P3,P4,P0,P2). The
safety algorithm is the standard Work/Finish simulation. requestResources()
checks Request <= Need and Request <= Available first (cheap rejection),
then tentatively commits the request, re-runs the safety algorithm, and
rolls the tentative commit back if the result is unsafe.
Two scenarios are run: P1 requesting (1,0,2), which is granted (a new safe
sequence exists), and P4 requesting (0,2,0) afterwards, which is denied —
not because it exceeds Need or Available (it doesn't), but because granting
it leaves no safe sequence, exercising the real rollback path rather than
the trivial bounds check.

Question 2 — Deadlock Detection via Wait-For Graph (deadlockdetect.c)
--------------------------------------------------------------------------
5 single-instance resources R0..R4, one owner each (alloc[r] = holder), and
a request[p] array (resource p is waiting for, or NONE). The wait-for edge
p -> q is added whenever p is waiting on a resource currently held by q.
Cycle detection is a standard DFS with a 3-colour visited array (white/gray/
black) and an explicit path stack, so when a back-edge to a gray node is
found, the exact cycle can be sliced out of the path and printed.
Scenario 1 is a 2-edge chain (P0->P1->P2) with no cycle. Scenario 2 adds a
third edge P2->P0, closing a 3-process cycle, alongside an unrelated P3->P4
edge that is waiting but not deadlocked, to show the detector doesn't
falsely flag ordinary waiting as deadlock.

Question 3 — Deadlock Prevention via Resource Ordering
-----------------------------------------------------------
Two real xv6 processes (fork()), two semaphores as binary locks (LOCK1,
LOCK2, sem_init'd to 1). resourceorder_bad.c has Process A take Lock1 then
try Lock2, while Process B takes Lock2 then tries Lock1, with a pause(10)
between each process's two acquisitions to force the overlap. Run with a
12-second bounded timeout: both processes print that they're trying their
second lock and then nothing else ever appears — genuinely hung on
sem_wait() with no timeout in the kernel, exactly the classic circular-wait
deadlock. The harness force-quits qemu (Ctrl-A x) once the timeout fires;
see output_logs/resourceorder_bad.txt and the screenshot for the exact
point it hangs.
resourceorder_fixed.c changes only the acquisition order: every process
takes Lock1 before Lock2, unconditionally. Now whichever process loses the
race for Lock1 just blocks on Lock1 (not a fresh lock the other process is
also waiting on), so there is no cycle in the wait-for graph — the same
workload completes cleanly every time (see output_logs/resourceorder_fixed.txt).
Why this eliminates circular wait: a deadlock's wait-for graph can only
contain a cycle if two processes wait on each other's held resources in
opposite orders. If every process is constrained to request locks in the
same total order, then whichever lock is "further along" that order can
never be held by a process that is itself blocked on a lock "earlier" in
the order — the wait-for edges can only ever point one direction along the
global order, so a cycle is structurally impossible.

Question 4 — Combined Synchronization & Deadlock Avoidance (syncdeadlock.c)
---------------------------------------------------------------------------
5 processes, 3 resource pools modelled directly as counting semaphores:
printer (2 instances), scanner (1 instance), disk (2 instances). Each
process needs 2 of the 3 types: P0=printer+scanner, P1=scanner+disk,
P2=printer+disk, P3=printer+scanner, P4=scanner+disk — chosen so the
single-instance scanner is contended by 4 of the 5 processes.
Strategy: resource ordering (the same technique as Q3, generalised to 3
resource types instead of 2 locks). Resources are given a fixed global
index — printer(0) < scanner(1) < disk(2) — and every process's pair is
already listed in that increasing order, so every process acquires its
lower-indexed resource first and its higher-indexed resource second,
regardless of which two types it happens to need. This is sufficient to
prevent circular wait for the same structural reason as Q3: no process
can be holding a higher-ordered resource while blocked on a lower-ordered
one that some other process holds while blocked on the first process's
resource — the wait-for graph can't close a loop.
Each process runs 3 cycles of request -> grant -> work (pause) -> release,
printing every step. The real run (output_logs/syncdeadlock.txt) shows
heavy contention on the scanner and correct blocking/serialization, but
all 5 processes finish all 3 cycles and the run ends with "All processes
completed, no deadlock" — no process is ever permanently blocked, and
sem_init's counts (2, 1, 2) guarantee the pools are never over-allocated.

Rebuild
-------
  1. Start from a pristine xv6-riscv with Assignment 5's kernel patches
     already applied (shm_get / sem_init / sem_wait / sem_signal) and its
     four user programs in user/.
  2. patch -p1 < diffs/Makefile.diff
  3. copy bankers.c, deadlockdetect.c, resourceorder_bad.c,
     resourceorder_fixed.c, syncdeadlock.c into user/
  4. make clean && make fs.img && make qemu CPUS=1
  5. run bankers / ddetect / rord_bad / rord_fix / syncdeadlock at the shell
     prompt (rord_bad is expected to hang — that's the point of Q3's first
     half; interrupt or wait out a timeout to get the shell back)
