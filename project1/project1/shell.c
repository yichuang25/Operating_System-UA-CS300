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
int number = 0;
char history[10][MAX_LINE];

void displayHistory() {
    printf("Shell Command History: \n");

    if(number <=10) {
        for(int i=0;i<record;i++) {
            printf("%d ", record-i);
            printf("%s\n",history[i]);
        }
    }
    else {
        for(int i=0;i<record;i++) {
            printf("%d ", number-i);
            printf("%s\n",history[i]);
        }
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
            printf("Error! Please enter an integer after !\n");
            return 1;
        }
        if(strcmp(args[0],"!!")==0) {
            if(record == 0) {
                printf("No command in history\n");
                return 1;
            }
            strcpy(inputBuffer,history[0]);
            indicate = 1;
        }
        else {
            for(int i=1;i<strlen(args[0]);i++) {
                if(args[0][i] < '0' || args[0][i] >'9') {
                    printf("Error, Please enter an integer after !\n");
                    return 1;
                }
            }
            char *integer = malloc(sizeof(strlen(args[0])-1));
            for(int i=1;i<strlen(args[0]);i++) {
                integer[i-1] = args[0][i];
            }
            int index = atoi(integer);
            if(index <= 0) {
                printf("Error, Please enter an positive integer after !\n");
                return 1;
            }
            if(number <= 10) {
                if(index <= 0 || index>number) {
                    printf("No such command in history\n");
                    return 1;
                }
                else {
                    strcpy(inputBuffer,history[record - index]);
                }
            }
            else {
                if(index<=number-10 || index>number) {
                    printf("No such command in history\n");
                    return 1;
                }
                else {
                    strcpy(inputBuffer,history[number-index]);
                }

            }
            
            indicate = 1;
            free(integer);
        }
        //printf("%s\n",inputBuffer);
    

        
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
        number++;
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
