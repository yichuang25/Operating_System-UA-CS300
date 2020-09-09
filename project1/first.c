#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char* argv[]) {
    
    int k = 0;

    pid_t pid;

    while(k<=0) {
        printf("Enter a positive number: \n");
        scanf("%d",&k);
        if(k <= 0) {
            printf("Error, please enter a positive number\n");
        }
    }

    pid = fork();

    if(pid == 0) {
        printf("Child is processing ...\n");
        printf("%d\n",k);
        while(k!=1) {
            if(k%2 == 0) {
                k = k/2;
            }
            else {
                k = 3*k + 1;
            }
            printf("%d\n",k);
        }
        printf("Child processing is done\n");
    }
    else {
        printf("Parent is waiting for child process...\n");
        wait(NULL);
        printf("parent processing is done\n");
    }

    return 0;
}