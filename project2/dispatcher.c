#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>

#define TIME_QUANTUM 1
#define BUFFER_SIZE 1024

enum process_state {ready, waiting};

typedef struct process {
    int arrival_time;
    int priority;
    int cpu_time;
    enum process_state state;
    pid_t pid;
} process;


typedef struct cda {
    bool flag; // identify queue
    //bool sorted;
    int size;
    int cap;
    int front;
    process *arr;
} CDA;

process* newProcess(int arrival, int priority, int time) {
    process *np = malloc(sizeof(process));
    np->arrival_time = arrival;
    np->priority = priority;
    np->cpu_time = time;
    np->pid = 0;
    np->state = ready;
    return np;
}

CDA *newCDA() {
    CDA *cda = (CDA *) malloc (sizeof(CDA));
    cda->flag = false;
    cda->size = 0;
    cda->front = 0;
    cda->cap =1;
    cda->arr = malloc(cda->cap * sizeof(process));
    return cda;
}

void copyProcess(process init,process *dest) {
    dest->arrival_time = init.arrival_time;
    dest->priority = init.priority;
    dest->cpu_time = init.cpu_time;
    dest->pid = init.pid;
    dest->state = init.state;
}

void doubleCapacity(CDA *cda) {
    int old_cap = cda->cap;
    int old_front = cda->front;
    process *temp = malloc(cda->cap * sizeof(process));
    for(int i=0;i<cda->size;i++) {
        copyProcess(cda->arr[(i+old_front)%old_cap],&temp[i]);
    }
    cda->cap = old_cap * 2;
    cda->front = 0;
    free(cda->arr);
    cda->arr = malloc(cda->cap * sizeof(process));
    for(int i=0;i<cda->size;i++) {
        copyProcess(temp[i],&cda->arr[i]);
    }
    free(temp);
}

void insertCDA_back(CDA *cda,process* np) {

    if(cda->cap == cda->size) {
        doubleCapacity(cda);
    }
    
    int last = (cda->front + cda->size)%cda->cap;

    //if(cda->size == 0) {
    //    cda->sorted = true;
    //}
    //else {
    //    if(cda->arr[(cda->front + cda->size - 1)%cda->cap].arrival_time > np->arrival_time && cda->flag == false) {
    //        cda->sorted = false;
    //    }
    //}
    copyProcess(*np,&cda->arr[last]);
    //cda->arr[last].arrival_time = np->arrival_time;
    //cda->arr[last].priority = np->priority;
    //cda->arr[last].cpu_time = np->cpu_time;
    //cda->arr[last].pid = np->pid;
    //cda->arr[last].state = np->state;
    cda->size++;

}
void halfCapacity(CDA *cda) {
    int oldfront = cda->front;
    int oldcap = cda->cap;
    process *temp = malloc(cda->size * sizeof(process));
    for(int i=0;i<cda->size;i++) {
        copyProcess(cda->arr[(oldfront+i)%oldcap],&temp[i]);
    }
    cda->cap = cda->cap/2;
    cda->front = 0;
    free(cda->arr);
    cda->arr = malloc(cda->cap * sizeof(process));
    for(int i=0;i<cda->size;i++) {
        copyProcess(temp[i],&cda->arr[i]);
    }
    free(temp);
}

void removeCDAfront(CDA *cda) {
    //printf("size: %d, Cap: %d\n",cda->size,cda->cap);
    if(cda->size <= 0) {
        return;
    }
    int newfront = (cda->front + 1) % cda->cap;
    cda->front = newfront;
    cda->size--;
    if(cda->cap>1 && (cda->size)/(double)cda->cap < 0.25) {
        //printf("Shrink\n");
        halfCapacity(cda);
    }
}

//void insertionSort(CDA *cda) {
//    int front = cda->front;
//    int capacity = cda->cap;
//    for(int i=1;i<cda->size;i++) {
//        for(int j=i-1;j>=0 && cda->arr[(front+j+1)%capacity].arrival_time < cda->arr[(front+j)%capacity].arrival_time;j--) {
//            process temp;
//            copyProcess(cda->arr[(front+j)%capacity],&temp);
//            copyProcess(cda->arr[(front+j+1)%capacity],&cda->arr[(front+j)%capacity]);
//            copyProcess(temp,&cda->arr[(front+j+1)%capacity]);
//        }
//    }
//    cda->sorted = true;
//}

void printCDA(CDA *cda) {
    for(int i=0;i<cda->size;i++) {
        printf("%d, %d, %d\n", cda->arr[(cda->front+i)%cda->cap].arrival_time,cda->arr[(cda->front+i)%cda->cap].priority,cda->arr[(cda->front+i)%cda->cap].cpu_time);
    }
}


void startProcess(process *p) {
    //printf("New process: %d, %d, %d\n",p->arrival_time,p->priority,p->cpu_time);
    
    pid_t child = fork();
    if(child == 0) {
        //printf("Ready process: %d, %d, %d\n",p->arrival_time,p->priority,p->cpu_time);
        int index = execvp("./process",NULL);
        printf("Execute Error: %d\n", index);
        printf("Fail process: %d, %d, %d\n",p->arrival_time,p->priority,p->cpu_time);
        exit(0);
    }
    else {
        p->pid = child;
    }
    
}

void suspendProcess (process *p) {
    //printf("Stop process: %d, %d, %d\n",p->arrival_time,p->priority,p->cpu_time);
    
    kill(p->pid,SIGTSTP);
    waitpid(p->pid,NULL,WUNTRACED);
    if(p->priority < 3)  {
        p->priority++; // insert to next priority
    }
    p->state = waiting;
}

void restartProcess(process *p) {
    //printf("Restart process: %d, %d, %d\n",p->arrival_time,p->priority,p->cpu_time);
    kill(p->pid,SIGCONT);
}

void terminateProcess(process *p) {
    //printf("Terminate process: %d, %d, %d\n",p->arrival_time,p->priority,p->cpu_time);
    kill(p->pid,SIGINT);
    waitpid(p->pid,NULL,WUNTRACED);
}

CDA *get_current_process(CDA *dq, int arrival) { // get the process arrived at this time
    CDA *list = newCDA();
    list->flag = true;
    for(int i=0;i<dq->size;i++) {
        process cur;
        copyProcess(dq->arr[i],&cur);
        if(cur.arrival_time == arrival) {
            insertCDA_back(list,&cur);
        }
    }
    return list;
}


int main (int argc, char *argv[]) {

    // Check input
    if(argc != 2) {
        printf("Require diaptach list\n");
        return 1;
    }

    FILE *dispatch_file = fopen(argv[1],"r");
    // check file
    if(dispatch_file == NULL) {
        printf("Can't open the file %s", argv[1]);
        return 1;
    }
    
    CDA *dispatch_queue = newCDA(); // create dispatch queue
    char line[BUFFER_SIZE];


    CDA **pq = malloc(sizeof(CDA *) * 4); // create priority queue
    for(int i=0;i<4;i++) {
        pq[i] = newCDA();
    }


    while(fgets(line,BUFFER_SIZE,dispatch_file)) {
        int arrival, priority, cpu_time; 
        
        sscanf(line,"%d,%d,%d",&arrival,&priority,&cpu_time); // get the docuemnt info line by line
        process *proc = newProcess(arrival,priority,cpu_time);
        if(priority<0 || priority >3) {
            printf("Invalid priority!\n");
        }
        else {
            insertCDA_back(dispatch_queue,proc); // add the process to dispatcher queue
        }
        
    }
    //if (dispatch_queue->sorted == false) {
    //    insertionSort(dispatch_queue);
    //}
    //printCDA(dispatch_queue);

    int curr_time = 0; // time stamp
    int number_process = 0; // process number

    CDA **rq = malloc(sizeof(CDA *) * 4); //create 4 priority queue
    
    for(int i=0;i<4;i++) {
        rq[i] = newCDA();
        rq[i]->flag = true;
    }

    process *currently_running = NULL;
    int sys_running = 0; // flag of system running
    int left = dispatch_queue->size; // size of the dispatcher list
    while(left > 0) {
        
        //printf("Second %d\n",curr_time);
        CDA *curr_process = get_current_process(dispatch_queue,curr_time);
        //printf("%d\n",curr_process->size);
        number_process = number_process + curr_process->size;
        for(int i=0;i<curr_process->size;i++) {
            process cur;
            copyProcess(curr_process->arr[i],&cur);
            insertCDA_back(rq[cur.priority],&cur);
        }
        //printf("Number of process: %d\n",number_process);
        //printf("Left process: %d\n",left);
        //for(int i=0;i<4;i++) {
        //    printf("\npriority queue: %d\n",i);
        //    printCDA(rq[i]);
        //    printf("\n");
        //}

        if(currently_running && currently_running->cpu_time == 0) { // check cputime for current process
            terminateProcess(currently_running);
            left--;
            if(sys_running) {
                sys_running = 0;

            }
            free(currently_running);
            currently_running = NULL;
            
        }

        if(rq[0]->size > 0 && !sys_running) { //system queue: first come, first serve
            if(currently_running) {
                suspendProcess(currently_running);
                insertCDA_back(rq[currently_running->priority],currently_running);
                //free(currently_running);
            }
            process *sys_pro = malloc(sizeof(process));
            copyProcess(rq[0]->arr[rq[0]->front],sys_pro);
            removeCDAfront(rq[0]);
            startProcess(sys_pro);
           
            if(currently_running) {
                free(currently_running);
            }
            currently_running = sys_pro;
            sys_running = 1;
        }
        else if(rq[1]->size > 0) { //priority 1 round robin
            if(currently_running) {
                suspendProcess(currently_running);
                insertCDA_back(rq[currently_running->priority],currently_running);
                //free(currently_running);
            }
            process *pone = malloc(sizeof(process));
            copyProcess(rq[1]->arr[rq[1]->front],pone);
            removeCDAfront(rq[1]);
            if(pone->state == ready) {
                startProcess(pone);
            }
            else {
                restartProcess(pone);
            }
            
            if(currently_running) {
                free(currently_running);
            }
            currently_running = pone;
            sys_running = 0;
            
        }
        else if(rq[2]->size > 0) { // priority 2
            if(currently_running) {
                suspendProcess(currently_running);
                //printf("current_running: %d, %d, %d\n",currently_running->arrival_time,currently_running->priority,currently_running->cpu_time);
                insertCDA_back(rq[currently_running->priority],currently_running);
                //printCDA(rq[2]);
                //printf("current_running: %d, %d, %d\n",currently_running->arrival_time,currently_running->priority,currently_running->cpu_time);
                //free(currently_running);
            }
            process *ptwo = malloc(sizeof(process));
            copyProcess(rq[2]->arr[rq[2]->front],ptwo);
            removeCDAfront(rq[2]);
            //printCDA(rq[2]);
            if(ptwo->state == ready) {
                startProcess(ptwo);
            }
            else {
                restartProcess(ptwo);
            }
            if(currently_running) {
                free(currently_running);
            }
            currently_running = ptwo;
            sys_running = 0;
            
        }
        else if(rq[3]->size > 0) { //last priority
            if(currently_running) {
                suspendProcess(currently_running);
                insertCDA_back(rq[currently_running->priority],currently_running);
                //free(currently_running);
            }
            process *pthree = malloc(sizeof(process));
            copyProcess(rq[3]->arr[rq[3]->front],pthree);
            removeCDAfront(rq[3]);
            if(pthree->state == ready) {
                startProcess(pthree);
            }
            else {
                restartProcess(pthree);
            }

            if(currently_running) {
                free(currently_running);
            }
            currently_running = pthree;
            sys_running = 0;
            
        }

        if(currently_running) {
            currently_running->cpu_time = currently_running->cpu_time - TIME_QUANTUM; //minus time quantum
        }

        curr_time++;
        free(curr_process);
        sleep(TIME_QUANTUM);
    }

    // Free the pointers
    for(int i=0;i<4;i++) {
        free(rq[i]->arr);
    }
    //free(dispatch_queue->arr);
    fclose(dispatch_file);

    wait(NULL);
    return 0;
}