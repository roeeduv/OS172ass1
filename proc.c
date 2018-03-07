#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;
/* assignment 1 - task 3 - this is the final change*/
int totalTickets = 0;	//total tickets for all proc
int schedMethod = 0;	//current sched method
int num=0;
int sum=0;
int seed=1;		//seed for random function
unsigned long
rand(unsigned long a) //random function
{
  return (a * 279470273UL) % 4294967291UL;
}
/* assignment 1 - task 3 */

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}


//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{

  struct proc *p;
  char *sp;
  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  if(p->state == UNUSED)
  goto found;

  release(&ptable.lock);
  return 0;

  found:
  p->state = EMBRYO;
  /* assignment 1 - task 3 */
  p->ctime = ticks;
  p->ttime = 0;
  p->stime = 0;
  p->rutime = 0;
  p->retime = 0;
  p->ztime = 0;
  p->priority=10; // use only in method 2
  p->ntickets = 20; //each procces starts with 20 tickets - use in method 2
  p->maxTickets=0;  //max 100 tickets to recive - use method 3
  if (schedMethod == 1 ){
    p->ntickets*=p->priority;
  }
  totalTickets +=  p->ntickets; //increase total tickets by ntickets of p
  //cprintf("new proc created with %d ntickets ! totalTickets is %d\n",p->ntickets,totalTickets);
/* assignment 1 - task 3 */
  p->pid = nextpid++;
  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;
  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();

  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
  panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;

  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
    return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
    return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
  if(proc->ofile[i])
  np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);

  safestrcpy(np->name, proc->name, sizeof(proc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.

/* assignment - part 2 - change code here -chang the signeture*/

void
exit(int status)
{

  struct proc *p;
  int fd;

  if(proc == initproc)
  panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }
  /* assignment 1 - task 2 */
	proc->exit_status = status;
  /* assignment 1 - task 2 */
  /* assignment 1 - task 3 */
	proc->ttime=ticks;
//     cprintf("termination - creation = %d - %d = %d  \n",proc->ttime, proc->ctime , proc->ttime-proc->ctime);
//      cprintf("sleep + ready + run  = %d + %d + %d  = %d \n",proc->stime, proc->retime,proc->rutime , proc->stime+proc->retime+proc->rutime);

  /* assignment 1 - task 3 */
  begin_op();
  iput(proc->cwd);
  end_op();
  proc->cwd = 0;

  acquire(&ptable.lock);
  /* assignment 1 - task 3 - change code */
  totalTickets -= proc->ntickets ; //decrease total tickets - proc is dead!
  /* assignment 1 - task 3 -finish change code */

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
      wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.


int
wait(int *status){

  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
      continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.

        /* assignment 1 - task 2 */
        *status = p->exit_status;
       /* assignment 1 - task 2 */
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

/*assignment - task3 */
void
methodUniform() // for policy 0 and 2 together
{
  struct proc *p;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){  //iterate all proc
    if(p->state == UNUSED )	//if unused do nothing
    continue;
    p->maxTickets=0;	//reset maxTickets for all procs
    if(p->state != SLEEPING ) {       //if not sleeping update totalTickets
     totalTickets = totalTickets - p->ntickets + 20;
    }
    p->ntickets = 20; 			//set 20 tickets for all proc
  }
  release(&ptable.lock);

}


void
methodPriority()    //for policy 1
{
  struct proc *p;
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){  //iterate all proc
    if(p->state == UNUSED )
    continue;
     if(p->state == SLEEPING ) {
    p->ntickets *= p->priority;                    //ntickets multiply by priority
    }
    else {
    totalTickets-=p->ntickets;
    p->ntickets *= p->priority;                    //ntickets multiply by priority
    totalTickets+=p->ntickets;
    }
  }
  release(&ptable.lock);
}


// adding system call policy - 1 */
void
policy(int policy_type)
{
  if (schedMethod == policy_type)
		cprintf("policy is already %d\n",policy_type);
  else {
    schedMethod = policy_type;
		if (policy_type ==  0 || policy_type ==  2 )
			methodUniform();
		if (policy_type ==  1 )
			methodPriority();
		if (policy_type < 0 || policy_type > 2 )
			cprintf("policy not in range , must be [1-3]\n");
  }
}

void
priority(int priority_number)
{
  if(schedMethod == 1)
  {
    totalTickets-=proc->ntickets;                           //update totalTickets
    proc->ntickets = proc->ntickets/proc->priority ;  //devide by priority to get regular amount
    proc->priority = priority_number;                 //update new priority
    proc->ntickets*=proc->priority ;                //compute new ntickets based on new priority
    totalTickets+=proc->ntickets; ;                         //update totalTickets
	//cprintf("priority changed to %d, totalTickets is now %d\n",priority_number,totalTickets);
  }
  else{
    cprintf("can't change priority, the method is : %d\n", schedMethod);
  }
}
/*assignment - task3 */

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  for(;;){
    // Enable interrupts on this processor.
    sti();
   /*assignment - task3 */
    seed++; //change seed each time  !
      if (totalTickets>0){ //if more than 1 proc
        num = rand(seed);
		//num*=ticks; //make random more random!
        num%=totalTickets;  //divide random by amount of tickets so num is between [0,totalTickets]
      }
      else{
        num=0; // no process
      }
	  if (num<0)
		num=num*-1;

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p+=1){
      if(p->state != RUNNABLE)
      continue;
      if(p->ntickets + sum < num) { // [0-p1.tickets] ,[p1.tickets-p1.tickets], [p2.tickets-p3.tickets] ...
         //update sum for next iteration
      	// cprintf("pid :%d name: %s ntickets:[%d - %d] rand num is: %d totalTickets: %d\n", p->pid , p->name,sum,p->ntickets+sum,num,totalTickets);
		 sum =sum+ p->ntickets;
		//cprintf("cont to next runnable proc!!\n");
		continue;
      }

    //  cprintf("running - pid :%d name: %s ntickets:[%d - %d] rand num is: %d totalTickets: %d\n", p->pid , p->name,sum,p->ntickets+sum,num,totalTickets);
      sum=0;
    /*assignment - task3 */

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      proc = p;
      switchuvm(p);
      p->state = RUNNING;
      swtch(&cpu->scheduler, p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      proc = 0;
    }
    release(&ptable.lock);
  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
  panic("sched ptable.lock");
  if(cpu->ncli != 1)
  panic("sched locks");
  if(proc->state == RUNNING)
  panic("sched running");
  if(readeflags()&FL_IF)
  panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;

  /*assignment - task3  */
  if (schedMethod == 2 ){
    if (proc->ntickets > 1){
      proc->ntickets-=1;
      totalTickets-=1;
    //  cprintf("dynamic yield new ntickets is : %d\n",proc->ntickets);
    //  cprintf("dynamic yield new totalTickets is : %d\n",totalTickets);
      if(proc->maxTickets != 0)
      proc->maxTickets--;
    }
  }
  /*assignment - task3 */
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
  panic("sleep");

  if(lk == 0)
  panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  /*assignment - task3 */
  if (schedMethod == 2 ){
    if (proc->maxTickets <= 90){
      proc->ntickets+=10;
      proc->maxTickets+=10;
      totalTickets += 10;
      //cprintf("dynamic sleep new maxTickets is : %d\n",proc->maxTickets);
     // cprintf("dynamic sleep new ntickets is : %d\n",proc->ntickets);
      //cprintf("dynamic sleep new totalTickets is : %d\n",totalTickets);
    }
    else {
      cprintf("proc can't get more than 100 tickets, maxTickets is : %d\n",proc->maxTickets);
    }
  }
  /*assignment - task3 */
    // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  /*assignment - task3 */
  totalTickets-=proc->ntickets;
  /*assignment - task3 */
  sched();

  // Tidy up.
  proc->chan = 0;
  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == SLEEPING && p->chan == chan){
      p->state = RUNNABLE;
	  /*assignment - task3 */
       totalTickets += p->ntickets;
     /*assignment - task3 */
	}
  }
}


// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
      p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
    [UNUSED]    "unused",
    [EMBRYO]    "embryo",
    [SLEEPING]  "sleep ",
    [RUNNABLE]  "runble",
    [RUNNING]   "run   ",
    [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
    continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
    state = states[p->state];
    else
    state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
      cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

/*assignment - task3 */
void
update_ticks()
{
  struct proc *p;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){ // loop over procs
    if(p->state == UNUSED || p==initproc)
    continue;
    if(p->state == RUNNING)
    {
        p->rutime+=1;
    }
    if(p->state == SLEEPING)
    {
        p->stime+=1;
    }
    if(p->state == RUNNABLE)
    {
          p->retime+=1;
    }
    if (p->state == ZOMBIE){
        p->ztime+=1;
  }
}
}
struct perf {
int ctime;
int ttime;
int stime;
int retime;
int rutime;
};

int
wait_stat(int* status, struct perf * performance){

  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
      continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.

        *status = p->exit_status;
        performance->ctime = p->ctime;
        performance->ttime = p->ttime;
        performance->stime = p->stime;
        performance->retime = p->retime;
        performance->rutime = p->rutime;

        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }
    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

/*assignment - task3 */
