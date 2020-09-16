/*
*   Yichen Huang
*   CWID:11906882
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAX_LINE 80 /* The maximum length command */
int record = 0;
char history[10][MAX_LINE];

void displayHistory() {
    printf("Shell Command History: \n");

    int i;
    int j = 0;
    
    for(int i=record-1;i>=0;i--) {
        printf("%d ", i+1);
        printf("%s\n",history[i]);
    }
}

int format(char inputBuffer[], char *args[], int *flag) {
    int length = 0;
    int first;
    int hist;

    fgets(inputBuffer,MAX_LINE,stdin);
    //inputBuffer = "ls &";
    //printf("%s\n", inputBuffer);
    length = strlen(inputBuffer);

    if (strcmp(inputBuffer,"exit\n") == 0) {
        printf("Goodbye\n");
        return 3;
    }

    
    if(length == 0) {
        printf("Error, command cannot read\n");
        return 1;
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
                args[count][i] = '\0';
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
                args[count][i] = '\0';
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

    //printf("%d\n",count);
    int indicate = 0;
    if(strcmp(args[0],"history") == 0) {
        if(record>0) {
            displayHistory();
        }
        else {
            printf("No Command In History\n");
        }
        return 1;
    }
    else if(args[0][0] == '!') {
        if(strlen(args[0]) == 1) {
            printf("No such command in history\n");
            return 1;
        }
        else if(strlen(args[0]) == 3) {
            //printf("2\n");
            int ten = args[0][1] - '0';
            int unit = args[0][2] - '0';
            //printf("%d, %d\n",ten,unit);
            if(ten == 1 && unit == 0) {
                //printf("3\n");
                strcpy(inputBuffer,history[9]);
                indicate = 1;
            }
            else {
                //printf("4\n");
                printf("No such command in history\n");
                return 1;
            }
        }
        else if(strlen(args[0]) == 2) {

            int unit = args[0][1] - '0';
            
            if(unit == -15){ // !!
            //printf("5\n");
                if(record == 0) {
                    printf("No command in history\n");
                    return 1;
                }
                strcpy(inputBuffer,history[0]);
                indicate = 1;
            }
            else if(unit > 0 && unit <= 9) {
                if(unit > record) {
                    //printf("6\n");
                    printf("No such command in history\n");
                    return 1;
                }
                else{
                    //printf("7\n");
                    strcpy(inputBuffer,history[unit-1]);
                    indicate = 1;
                }
            }
            
        }
        
    }

    if(indicate == 0) {
        //printf("Add to history...\n");
        for(int i=9;i>0;i--) {
            strcpy(history[i],history[i-1]);
        }
        strcpy(history[0],inputBuffer);
        record++;
        if(record>10) {
            record = 10;
        }
    }
 
   
    if(indicate == 1) {
        //printf("Changing buffer %s",inputBuffer);
        //printf("%lu\n",strlen(inputBuffer));
        for(int i=count-1;i>=0;i--) {
            free(args[i]);
        }

        start = -1;
        count = 0;
        length =strlen(inputBuffer);

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
                    args[count][i] = '\0';
                    ++count;
                }
                start = -1;
            }
            else if (inputBuffer[i] == '\n' || inputBuffer[i] == '\0') {
                if(start != -1) {
                    char temp[i-start+1];
                    for(int j=start;j<i;j++) {
                        temp[j-start] = inputBuffer[j];
                    }
                    args[count] = malloc(sizeof(temp));
                    for(int k=0;k<strlen(temp);k++) {
                        args[count][k] = temp[k];
                    }
                    args[count][i] = '\0';

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
        //printf("%d\n",count);

    }
    //printf("args[]\n");
    //for(int i=0;i<count;i++) {
    //    printf("%s ",args[i]);
    //}
    //printf("\n");

    
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
                //printf("Child processing...\n");
                if(execvp(args[0],args) == -1) {
                    printf("Error executing command\n");
                    exit(0);
                    
                }
                //printf("Child processing done...\n");
            }

            else {
                //printf("Parent processing starting\n");
                if(flag == 0) {
                    //printf("Waiting for child finish...\n");
                    wait(NULL);
                }
                //printf("Parent processing done\n");
            }

        }
        else if (index == 3) {
            should_run = 0;
        }
        
    }
    return 0;
}
