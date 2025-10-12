#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


// Simple check macro: print PASS/FAIL, but avoid calling inside critical measurements
#define CHECK(tag, cond, expect_str, actual) \
  do { \
    if (cond) { \
      printf("[PASS] %s: %s -> %d\n", tag, expect_str, (actual)); \
    } else { \
      printf("[FAIL] %s: %s -> %d\n", tag, expect_str, (actual)); \
    } \
  } while (0)

static int pages_for_bytes(int bytes) {
  // Same calculation as kernel page_usage: round up
  const int PGSIZE = 4096;
  return (bytes + PGSIZE - 1) / PGSIZE;
}

static void test_syscall_counter_no_printf_gap(void) {
  // Note: do not insert printf between two sysinfo(1) and getpid()
  int a = sysinfo(1);
  int dummy = getpid(); (void)dummy;
  int b = sysinfo(1);

  // Expectation: b == a + 1 (only one getpid, sysinfo(1) does not count itself)
  // If implementation increments after sys_* call, it might become b >= a + 1
  CHECK("sysinfo(1) global syscall count",
        (b == a + 1) || (b > a), "after one getpid, count >=", b);
}

static void test_active_process_count_with_fork(void) {
  int p0 = sysinfo(0);  // before fork

  int cpid = fork();
  if (cpid < 0) {
    printf("[FAIL] fork failed\n");
    return;
  }

  if (cpid == 0) {
    // child: stay alive for a while so parent can observe it as active
    sleep(50);
    exit(0);
  } else {
    // parent
    sleep(1); // give scheduler some time
    int p1 = sysinfo(0);
    // Expect at least +1 (new child process), possibly more depending on states
    CHECK("sysinfo(0) active procs after fork", (p1 >= p0 + 1), ">= p0+1", p1);

    wait(0);
    int p2 = sysinfo(0);
    // Expect decrease (at least -1)
    CHECK("sysinfo(0) after wait", (p2 <= p1 - 1), "<= p1-1", p2);
  }
}

static void test_free_pages_decrease_after_sbrk(int bytes) {
  int f0 = sysinfo(2);

  // Request bytes of memory
  void *old = sbrk(bytes);
  if (old == (void*)-1) {
    printf("[FAIL] sbrk failed for %d bytes\n", bytes);
    return;
  }

  int f1 = sysinfo(2);
  int need_pages = pages_for_bytes(bytes);
  int delta = f0 - f1;

  // Expect at least need_pages (may be more due to metadata/alignment)
  CHECK("sysinfo(2) free pages after sbrk",
        (delta >= need_pages), "delta >= need_pages", delta);
}

static void test_procinfo_in_child(void) {
  int parent_pid = getpid();
  int cpid = fork();
  if (cpid < 0) {
    printf("[FAIL] fork failed\n");
    return;
  }

  if (cpid == 0) {
    // child
    struct pinfo pi1, pi2;

    // First procinfo read, no printf in between
    int r1 = procinfo(&pi1);
    // Trigger an extra syscall (getpid)
    int gp = getpid(); (void)gp;
    int r2 = procinfo(&pi2);

    // Check return values
    CHECK("procinfo copyout success #1", (r1 == 0), "r1 == 0", r1);
    CHECK("procinfo copyout success #2", (r2 == 0), "r2 == 0", r2);

    // Check ppid: should equal parent pid
    CHECK("procinfo.ppid == parent pid", (pi1.ppid == parent_pid), "ppid == parent", pi1.ppid);

    // Check syscall_count: second > first (at least +1, excluding current procinfo)
    CHECK("procinfo.syscall_count increased",
          (pi2.syscall_count > pi1.syscall_count),
          "pi2 > pi1", pi2.syscall_count);

    // Check page_usage: should be positive
    CHECK("procinfo.page_usage positive",
          (pi1.page_usage > 0 && pi2.page_usage > 0),
          "> 0", pi2.page_usage);

    exit(0);
  } else {
    wait(0);
  }
}

int
main(int argc, char *argv[])
{
  // Default: request 64 KiB, same as lab example
  int bytes = 65536;
  if (argc >= 2) {
    bytes = atoi(argv[1]);
    if (bytes <= 0) bytes = 65536;
  }

  printf("=== Lab1 comprehensive test start (mem=%d bytes) ===\n", bytes);

  // 1) Global syscall counter: insert one getpid()
  test_syscall_counter_no_printf_gap();

  // 2) Active processes: fork/sleep/wait
  test_active_process_count_with_fork();

  // 3) Free pages: sbrk(bytes)
  test_free_pages_decrease_after_sbrk(bytes);

  // 4) Child procinfo check (ppid/syscall_count/page_usage)
  test_procinfo_in_child();

  // Extra: print current sysinfo overview
  int n_active = sysinfo(0);
  int n_syscalls = sysinfo(1);
  int n_free = sysinfo(2);
  printf("[summary] active=%d, syscalls=%d, free_pages=%d\n",
         n_active, n_syscalls, n_free);

  printf("=== Lab1 comprehensive test done ===\n");
  exit(0);
}
