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

int main(int argc, char *argv[])
{
    // test_sysinfo(argc, argv);
    // test_sysinfo_2();
    test_procinfo();
    exit(0);
}