/*
*
*   Yichen Huang
*   CS-300-001
*   11906882
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define INPUT_SIZE 8
#define FRAME_SIZE 256
#define FRAME_NUMBER 256
#define TLB 16
#define PAGE_TABLE 256

bool pageTableValid[PAGE_TABLE];
int pageTableFrames[PAGE_TABLE];

int TLB_Page[TLB]; //page number in TLB
int TLB_Frame[TLB]; //Frame number in TLB

int physicalMemory[FRAME_NUMBER][FRAME_SIZE];

int Pagetable_Avaliable = 0;
int First_Free_Frame = 0;
int TLB_Counter = 0;

FILE *backing_store;
char buffer[256]; //hold input from backing store

void readFromstore(int pageNumber) {
    if(fseek(backing_store,pageNumber*256,SEEK_SET) != 0) {
        printf("Error seeking from backing store\n");
    }

    if(fread(buffer,sizeof(char),256,backing_store) == 0) {
        printf("Error reading from backing store\n");
    }
}

void insertTLB(int pageNumber, int frameNumber){
    for(int i=0;i<TLB_Counter;i++) {
        if(TLB_Page[i] == pageNumber) {
            break;
        }
    }

    TLB_Page[TLB_Counter] = pageNumber;
    TLB_Frame[TLB_Counter] = frameNumber;

    TLB_Counter = (TLB_Counter + 1) % TLB;
}

int main(int argc, char *argv[]) {

    if(argc!=2) {
        printf("Usage: input file\n");
        return 1;
    }

    //open backing store
    backing_store = fopen("BACKING_STORE.bin","rb");
    if(backing_store == NULL) {
        printf("Error opening backing store\n");
        return 1;
    }

    //open address file
    FILE *address_file = fopen(argv[1],"r");
    if(address_file == NULL) {
        printf("Error opening address file\n");
        return 1;
    }

    int TLB_Hits = 0;
    int pageFault = 0;
    int addressCounter = 0;
    char address[INPUT_SIZE];
    
    for(int i=0;i<PAGE_TABLE;i++) { //reset page table
        pageTableFrames[i] = -1;
        pageTableValid[i] = false;
    }

    for(int i=0;i<TLB;i++) { //reset TLB
        TLB_Page[i] = -1;
        TLB_Frame[i] = -1;
    }

    while(fgets(address,INPUT_SIZE,address_file) != NULL) { //reading address
        int logical_address = atoi(address);
        //printf("%d\n", logical_address);
        int pageNumber = ((logical_address & 0xFFFF)>>8);
        //printf("PageNumber: %d\n", pageNumber);
        int offset = (logical_address & 0xFF);
        //printf("Offset: %d\n", offset);
        int frameNumber = -1;

       
        for(int i=0;i<TLB;i++) { //TLB hit
            if(TLB_Page[i] == pageNumber) {
                frameNumber = TLB_Frame[i];
                TLB_Hits++;
            }
        }

        if(frameNumber == -1) { //TLB miss
            if(pageTableValid[pageNumber]) {
                frameNumber = pageTableFrames[pageNumber];
            }

            if(frameNumber == -1) { //page fault
                readFromstore(pageNumber);
                for(int i=0;i<256;i++) {
                    physicalMemory[First_Free_Frame][i] = buffer[i];
                }
                pageTableFrames[pageNumber] = First_Free_Frame;
                pageTableValid[pageNumber] = true;
                First_Free_Frame++;
                pageFault++;
                frameNumber = First_Free_Frame - 1;
            }
            insertTLB(pageNumber,frameNumber);
        }
        
        int value = physicalMemory[frameNumber][offset];
        printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, (frameNumber << 8) | offset, value);
        addressCounter++;
    }

    //statistics
    printf("Number of translated addresses = %d\n", addressCounter);
    double pfRate = pageFault / (double)addressCounter;
    double TLBRate = TLB_Hits / (double)addressCounter;
    printf("Page Faults = %d\n", pageFault);
    printf("Page Fault Rate = %.3f\n",pfRate);
    printf("TLB Hits = %d\n", TLB_Hits);
    printf("TLB Hit Rate = %.3f\n", TLBRate);



    //close file
    fclose(backing_store);
    fclose(address_file);
    return 0;
}