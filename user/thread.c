// lab3
#include "kernel/types.h"
#include "user/thread.h"
#include "kernel/riscv.h"
#include "user/user.h"

int thread_create(void *(start_routine)(void*), void *arg) {
  void *stack = sbrk(PGSIZE);
  if (stack == 0) {
    return -1; // failed to allocate stack
  }

  int tid = clone(stack);
  if (tid < 0) {
    return -1; // clone failed; can not "free(stack)" due to sbrk implementation
  }

  if (tid == 0) {
    // Child thread
    start_routine(arg);
    exit(0);
  }

  return 0; // parent thread returns 0
}

void lock_init(struct lock_t* lock) {
  lock->locked = 0;
}

void
lock_acquire(struct lock_t *lock)
{
  // actually, this version has been deprecated. Using __atomic_test_and_set is better.
  while (__sync_lock_test_and_set(&lock->locked, 1) != 0) {
    // spin-wait (do nothing)
  }
}

// recommended version using C11 atomic built-ins
// void lock_acquire(struct lock_t* lock) {
//   while (__atomic_test_and_set(&lock->locked, __ATOMIC_ACQUIRE)) {
//   }
// }

void
lock_release(struct lock_t *lock)
{
    __sync_lock_release(&lock->locked);
}


