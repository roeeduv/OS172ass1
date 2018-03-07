#include "types.h"
#include "user.h"


/* test for 3*/
void fib(int n) {
	if (n <= 1)
		return;
	fib(n-1);
	fib(n-2);
}

int main() {
	fib(39);
	exit(0);
}


/* test for 2.1, 2.2
int main() {
    int status;
    if (fork() >0) {
        wait(&status);
        if (status != 137) {
            printf(1, "Fail\n");
        } else {
            printf(1, "Passed\n");
        }
        exit(0);
    } else {
        exit(137);
    }
}
*/