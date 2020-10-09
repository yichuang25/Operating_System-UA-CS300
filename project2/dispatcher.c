#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>

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
    bool sorted;
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
    for(int i=0;i<cda->cap;i++) {
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

    if(cda->size == 0) {
        cda->sorted = true;
    }
    else {
        if(cda->arr[(cda->front + cda->size - 1)%cda->cap].arrival_time > np->arrival_time) {
            cda->sorted = false;
        }
    }

    cda->arr[last].arrival_time = np->arrival_time;
    cda->arr[last].priority = np->priority;
    cda->arr[last].cpu_time = np->cpu_time;
    cda->arr[last].pid = np->pid;
    cda->arr[last].state = np->state;
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
    if(cda->size <= 0) {
        return;
    }
    if(cda->cap>1 && (cda->size-1)/cda->cap < 0.25) {
        halfCapacity(cda);
    }
    int newfront = (cda->front + 1 + cda->size) % cda->cap;
    cda->front = newfront;
    cda->size--;
}

void insertionSort(CDA *cda) {
    int front = cda->front;
    int capacity = cda->cap;
    for(int i=1;i<cda->size;i++) {
        for(int j=i-1;j>=0 && cda->arr[(front+j+1)%capacity].arrival_time < cda->arr[(front+j)%capacity].arrival_time;j--) {
            process temp;
            copyProcess(cda->arr[(front+j)%capacity],&temp);
            copyProcess(cda->arr[(front+j+1)%capacity],&cda->arr[(front+j)%capacity]);
            copyProcess(temp,&cda->arr[(front+j+1)%capacity]);
        }
    }
    cda->sorted = true;
}

void printCDA(CDA *cda) {
    for(int i=0;i<cda->size;i++) {
        printf("%d, %d, %d\n", cda->arr[(cda->front+i)%cda->cap].arrival_time,cda->arr[(cda->front+i)%cda->cap].priority,cda->arr[(cda->front+i)%cda->cap].cpu_time);
    }
}


void runprocess(process *p) {
    pid_t child = fork();
    if(child == 0) {
        char **command = malloc(sizeof(char *));
        command[0] = "./process";
        execvp(command[0],command);
        free(command);
    }
    else {
        p->pid = child;
    }
}

void stopprocess (process *p) {
    kill(p->pid,SIGSTOP);
    waitpid(p->pid,NULL,WUNTRACED);
    if(p->priority < 3)  {
        p->priority++;
    }
    p->state = waiting;
}

void continueprocess(process *p) {
    kill(p->pid,SIGCONT);
}

void terminate(process *p) {
    kill(p->pid,SIGINT);
    waitpid(p->pid,NULL,WUNTRACED);
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
        int count = sscanf(line,"%d,%d,%d",&arrival,&priority,&cpu_time);
        process *proc = newProcess(arrival,priority,cpu_time);
        insertCDA_back(dispatch_queue,proc);
    }
    if (dispatch_queue->sorted == false) {
        insertionSort(dispatch_queue);
    }
    printCDA(dispatch_queue);

    int curr_time = 0;
    int number_process = 0;

    CDA **rq = malloc(sizeof(CDA *) * 4); //create 4 priority queue
    
    for(int i=0;i<4;i++) {
        rq[i] = newCDA();
    }

    process *currently_running = NULL;
    int sys_running = 0;
    while(currently_running || number_process != dispatch_queue->size) {
        
    }

    return 0;
}