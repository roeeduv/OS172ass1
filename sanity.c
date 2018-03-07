#include "fcntl.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
struct perf {
int ctime;
int ttime;
int stime;
int retime;
int rutime;
};

void fib(int n) {
	if (n <= 1)
		return;
	fib(n-1);
	fib(n-2);
}

void
sanity_test(int policy_number)
{
        struct perf child_perf[10];
        policy(policy_number);
        for(int i = 0; i < 10 ; i++)
        {
            if(fork() == 0){
                fib(30);
                exit(i);
            }
        }
				for(int i = 0 ; i < 10 ; i++)
				{
					int status,pid;
					struct perf *performance = malloc(5*sizeof(int));
					pid = wait_stat(&status,performance);
					child_perf[i].ctime = performance->ctime;
					child_perf[i].ttime = performance->ttime;
					child_perf[i].stime = performance->stime;
					child_perf[i].retime = performance->retime;
					child_perf[i].rutime = performance->rutime;
					printf(1,"child number:%d return\n", pid);
					free(performance);
				}


        /* know calculate*/
        int turnaround_avg = 0;
        int wait_avg = 0;
        for(int i = 0; i < 10 ; i++)
        {
            turnaround_avg = turnaround_avg + (child_perf[i].ttime - child_perf[i].ctime);
            // printf(1,"child number %d was alive for %d\n",i ,(child_perf[i].ttime - child_perf[i].ctime));
            // printf(1, "sleep + ready + run  = %d + %d + %d  = %d \n",child_perf[i].stime, child_perf[i].retime,child_perf[i].rutime , child_perf[i].stime+child_perf[i].retime+child_perf[i].rutime);
        }
        turnaround_avg = turnaround_avg/10;
        printf(1,"Turnaround for policy number %d is : %d\n",policy_number,turnaround_avg);

        for(int i = 0; i < 10 ; i++)
        {
            wait_avg = wait_avg + (child_perf[i].retime);
            // printf(1,"child number %d was waiting for %d\n",i,child_perf[i].retime);
        }
        wait_avg = wait_avg/10;
        printf(1,"wait for policy number %d is : %d\n", policy_number,wait_avg);
        return;
}

void
sanity_test_1(int policy_number)
{
        struct perf child_perf[10];
        policy(policy_number);
        for(int i = 0; i < 10 ; i++)
        {
            if(fork() == 0){
							priority(i);
                fib(30);
                exit(i);
            }
        }
				for(int i = 0 ; i < 10 ; i++)
				{
					int status,pid;
					struct perf *performance = malloc(5*sizeof(int));
					pid = wait_stat(&status,performance);
					child_perf[i].ctime = performance->ctime;
					child_perf[i].ttime = performance->ttime;
					child_perf[i].stime = performance->stime;
					child_perf[i].retime = performance->retime;
					child_perf[i].rutime = performance->rutime;
					printf(1,"child number:%d return\n", pid);
					free(performance);
				}


        /* know calculate*/
        int turnaround_avg = 0;
        int wait_avg = 0;
        for(int i = 0; i < 10 ; i++)
        {
            turnaround_avg = turnaround_avg + (child_perf[i].ttime - child_perf[i].ctime);
            // printf(1,"child number %d was alive for %d\n",i ,(child_perf[i].ttime - child_perf[i].ctime));
            // printf(1, "sleep + ready + run  = %d + %d + %d  = %d \n",child_perf[i].stime, child_perf[i].retime,child_perf[i].rutime , child_perf[i].stime+child_perf[i].retime+child_perf[i].rutime);
        }
        turnaround_avg = turnaround_avg/10;
        printf(1,"Turnaround for policy number %d is : %d\n",policy_number,turnaround_avg);

        for(int i = 0; i < 10 ; i++)
        {
            wait_avg = wait_avg + (child_perf[i].retime);
            // printf(1,"child number %d was waiting for %d\n",i,child_perf[i].retime);
        }
        wait_avg = wait_avg/10;
        printf(1,"wait for policy number %d is : %d\n", policy_number,wait_avg);
        return;
}
int main() {
   sanity_test(0);
   sanity_test_1(1);
   sanity_test(2);
   exit(0);

}
