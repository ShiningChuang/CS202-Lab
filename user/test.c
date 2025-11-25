#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void test_hello(int argc, char *argv[]) {
    int n = 0;
    if(argc >= 2)
        n = atoi(argv[1]);
    printf("Say hello to kernel %d\n", n);
    hello(n); // call hello syscall
}

void test_sysinfo(int argc, char *argv[]) {
    int n = 0;
    if(argc >= 2)
        n = atoi(argv[1]);
    int result = sysinfo(n); // call sysinfo syscall
    printf("Result from sysinfo(%d): %d\n", n, result);
}

void test_sysinfo_1() {
    int a = sysinfo(1);
    int b = sysinfo(1);
    printf("%d -> %d (expect +1)\n", a, b);
    int c = sysinfo(1);
    printf("%d -> %d (after printing)\n", b, c);
}

void test_sysinfo_2() {
    printf("free pages before malloc: %d\n", sysinfo(2));
    malloc(10000);
    printf("free pages after malloc: %d\n", sysinfo(2));
}

void test_procinfo() {
    struct pinfo pi;
    if (procinfo(&pi) == -1) {
        printf("procinfo failed!\n");
        exit(1);
    }
    printf("ppid: %d, syscall_count: %d, page_usage: %d\n", pi.ppid, pi.syscall_count, pi.page_usage);
}

void test_sched_statistics() {
    int result = sched_statistics(); // call sched_statistics syscall
    printf("Result from sched_statistics: %d\n", result);
}

void test_tickets(int n) {
    int result = sched_tickets(n); // call tickets syscall
    printf("Set tickets to %d, result: %d\n", n, result);
    test_sched_statistics();
}

void test_clone() {
    void *stack = malloc(4096);
    int tid = clone(stack);
    printf("Clone returned thread id: %d\n", tid);
}

void test_clone_basic() {
    void *stack = malloc(4096);
    int tid = clone(stack);

    if (tid < 0) {
        printf("clone failed!\n");
        exit(1);
    }

    if (tid == 0) {
        // child thread path
        printf("[child] I am the child thread! tid=0\n");
        exit(0);
    } else {
        // parent
        printf("Clone returned thread id: %d\n", tid);
        wait(0);
        printf("Parent done, now exiting\n");
        exit(0);
    }
}
int recursion(int x) {
    if (x == 0) return 0;
    return 1 + recursion(x - 1);
}

void test_clone_stack() {
    void *stack = malloc(4096);
    int tid = clone(stack);

    if (tid == 0) {
        // child
        int x = recursion(20);
        printf("[child] recursion returned %d (stack OK)\n", x);
        exit(0);
    } else {
        printf("[parent] tid = %d created\n", tid);
        wait(0);
    }
}

void test_clone_multiple() {
    for (int i = 0; i < 3; i++) {
        void *stack = malloc(4096);
        int tid = clone(stack);

        if (tid == 0) {
            printf("[child] hello, I am thread\n");
            exit(0);
        }
    }
    for (int i = 0; i < 3; i++) wait(0);
}

void test_clone_wait() {
    void *stack1 = malloc(4096);
    void *stack2 = malloc(4096);

    int tid1 = clone(stack1);
    if (tid1 == 0) {
        printf("[child1] exit\n");
        exit(0);
    }

    int tid2 = clone(stack2);
    if (tid2 == 0) {
        printf("[child2] exit\n");
        exit(0);
    }

    int pid;
    while ((pid = wait(0)) > 0) {
        printf("[parent] collected child pid %d\n", pid);
    }
}

void test_clone_work() {
    void *stack = malloc(4096);
    int tid = clone(stack);

    if (tid == 0) {
        for (int i = 0; i < 5; i++) {
            printf("[child] i=%d\n", i);
        }
        exit(0);
    } else {
        wait(0);
        printf("[parent] done\n");
    }
}

int main(int argc, char *argv[])
{
    test_clone_work();
    exit(0);
}