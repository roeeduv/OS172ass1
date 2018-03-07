#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

/*assignmtnt1 - task3 - change code here - adding system call policy - 3 */
 int
sys_wait_stat(void)
{
  int status,performance;
  if (argint(0, &status) < 0)
    return -1;
  /*  if (argptr(1, (char**)&performance,sizeof(struct perf*)) < 0)
      return -1;
      */
      if (argint(1, &performance) < 0)
        return -1;
  return wait_stat((int *)status,(struct perf *)performance);
}
int
sys_policy(void)
{
    int policy_number;
    if (argint(0,&policy_number) < 0)
      return -1;
    policy(policy_number);
    return 0;  // not reached
}


int
sys_priority(void)
{
  int priority_number;
  if (argint(0,&priority_number) < 0)
    return -1;
  priority(priority_number);
  return 0;  // not reached
}
/* finish changinc code*/

int
sys_fork(void)
{
  return fork();
}
/*assignmtnt1 - part 2 - change code here*/
int
sys_exit(void)
{
  int status;
  if (argint(0,&status) < 0)
    return -1;
  exit(status);
  return 0;  // not reached
}
/* assignmtnt1 - part 2 - change code here*/
int
sys_wait(void)
{
  int status;
  if (argint(0, &status) < 0)
    return -1;
  return wait((int *)status);
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
