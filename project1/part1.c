#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LINE 80 /* The maximum length command */
int count = 0;

void displayHistory() {
    printf("Shell Command History: \n");

    int i;
    int j = 0;
    int hisCount = count;

    for(int i=0;i<10;i++) {
        
    }
}

int format(char inputBuffer[], char *args[], int *flag) {
    int length = 0;
    int first;
    int history;

    fgets(inputBuffer,MAX_LINE,stdin);
    //inputBuffer = "\n";
    //printf("%s\n", inputBuffer);
    length = strlen(inputBuffer);

    if (strcmp(inputBuffer,"exit\n") == 0) {
        printf("Goodbye\n");
        return 2;
    }

    
    if(length == 0) {
        printf("Error, command cannot read\n");
        exit(1);
    }

    int start = -1;
    int count = 0;

    for(int i=0;i<length;i++) {
        if(inputBuffer[i] == ' ' || inputBuffer[i] == '\t') {
            if(start != -1) {
                char temp[i-start+1];
                for(int j=start;j<i;j++) {
                    temp[j-start] = inputBuffer[j];
                }
                args[count] = malloc(sizeof(temp));
                for(int k=0;k<strlen(temp);k++) {
                    args[count][k] = temp[k];
                }
                ++count;
            }
            start = -1;
        }
        else if (inputBuffer[i] == '\n') {
            if(start != -1) {
                char temp[i-start+1];
                for(int j=start;j<i;j++) {
                    temp[j-start] = inputBuffer[j];
                }
                args[count] = malloc(sizeof(temp));
                for(int k=0;k<strlen(temp);k++) {
                    args[count][k] = temp[k];
                }

                ++count;
            }
            
            args[count] = NULL;
        }
        else {
            if(start == -1) {
                start = i;
            }

            if(inputBuffer[i] == '&') {
                start = -1;
                *flag = 1;
                
            }
        }
    }
    args[count] = NULL;
    if(args[0] == NULL) {
        return 1;
    }

    if(strcmp(args[0],"history") == 0) {
        if(count>0) {
            displayHistory();
        }
        else {
            printf("\nNo Command In History\n");
        }
        return -1;
    }
    else if(args[0][0] == '!') {
        if(strlen(args[0]) == 1) {
            return -1;
        }
        int x = args[0][1]-'0';
        int y =0;
        
    }




    
    return 0;
}


int main(void) {
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    char command[MAX_LINE];
    int flag;
    int should_run = 1; /* flag to determine when to exit program */
    pid_t pid;
    while (should_run) {
        flag = 0;
        printf("osh>");
        fflush(stdout);
        int index = format(command, args, &flag);
        if(index == 0) {
            pid = fork();
            if(pid < 0) {
                printf("Fork Failed\n");
                exit(1);
            }

            else if(pid == 0) {
                if(execvp(args[0],args) == -1) {
                    printf("Error executing command\n");
                }
            }

            else {
                if(flag == 0) {
                    wait(NULL);
                }
            }

        }
        else if (index == 2) {
            should_run = 0;
        }
        
    }
    return 0;
}