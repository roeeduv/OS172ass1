#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

/* test for the schedular*/
unsigned long rand(unsigned long a)
{
  return (a * 279470273UL) % 4294967291UL;
}

int
main(int argc, char *argv[])
{
  int i, num=1;
  int arr[]={10,10,10,10,10};

  //if(argc < 2){
   // ls(".");
  //}
  for(i=5; i<15; i++){
    //ls(argv[i]);
    num = rand(i);
    num=num%50;
    if (num<0)
    num=num*-1;
  printf(1, "num: %d\n",num);
  }
  int sum=0;
  for (int i=0;i<5;i++){
   if(arr[i] + sum <= num) { // [0-p1.tickets] ,[p1.tickets-p1.tickets], [p2.tickets-p3.tickets] ...
        	sum += arr[i];//update sum for next iteration
        	continue;
    }
    printf(1, "index is: %d\n",i); //first index printed is the right one!
  }
   printf(1, "sum is: %d\n",sum);


}
