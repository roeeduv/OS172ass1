#include "fcntl.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int main() {
    
    int policy_number;
    read(0, &policy_number, 1);
    printf(1, "%d\n" , (policy_number - 48));
    policy(policy_number - 48);
    return 0;

}
 
