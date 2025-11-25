#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_hello(void) // hello system call definition
{
  int n;
  argint(0, &n);
  print_hello(n);
  return 0;
}

uint64
sys_sysinfo(void) // info system call definition
{
  int n;
  int result;
  argint(0, &n);
  if (n == 0) { // get total number of active processes
    result = get_active_processes_num();
  } else if (n == 1) {  // get total number of system calls not including this time
    result = get_syscall_num()-1; 
  } else if (n == 2) {  // get the number of free memory pages
    result = get_free_memory_pages_num(); 
  } else {  // invalid argument
    printf("[K_INFO] invalid argument %d\n", n);
    result = -1; 
  }
  return result;
}


// per process info for procinfo syscall, mirror struct pinfo using in kernel
struct pinfo_kernel {
  int ppid;
  int syscall_count;
  int page_usage;
};

uint64
sys_procinfo(void) // procinfo system call definition
{
  printf("[K_INFO] This is procinfo \n");
  struct proc *p = myproc();
  struct pinfo_kernel pinfo_k;
  uint64 user_addr;
  argaddr(0, &user_addr); // no need to check, copyin/copyout will do that

  acquire(&p->lock);
  pinfo_k.ppid = (p->parent) ? p->parent->pid : -1;
  pinfo_k.syscall_count = p->current_proc_syscall_num - 1; // not including this time
  pinfo_k.page_usage = (p->sz + PGSIZE - 1) / PGSIZE; // round up, 10000 bytes -> 3 pages
  release(&p->lock);

  // copyout(pagetable_t pagetable, uint64 dstva, char *src, uint64 len), return 0 on success, -1 on error.
  int res = copyout(p->pagetable, user_addr, (char *)&pinfo_k, sizeof(pinfo_k));
  if (res < 0) {
    printf("[K_INFO] procinfo copyout failed \n");
  } else {
    printf("[K_INFO] procinfo copyout success \n");
  }
  return res;
}

//lab2
uint64
sys_sched_statistics(void) // sched_statistics system call definition
{
  printf("[K_INFO] This is sched_statistics \n");
  return sched_statistics();
}

//lab2
uint64
sys_sched_tickets(void) // tickets system call definition
{
  int n;
  argint(0, &n);
  printf("[K_INFO] This is tickets");
  return set_tickets(n);
}

//lab3
uint64
sys_clone(void) // clone system call definition
{
  uint64 stack;
  argaddr(0, &stack);
  printf("[K_INFO::sys_clone] stack: %d \n", stack);
  return clone(stack);
}


