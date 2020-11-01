#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define INPUT_SIZE 8
#define FRAME_SIZE 256
#define FRAME_NUMBER 128
#define TLB 16
#define PAGE_TABLE 256

int pageTableFrames[PAGE_TABLE];
bool pageTableValid[PAGE_TABLE];

int TLB_Page[TLB];
int TLB_Frame[TLB];

int physicalMemory[FRAME_NUMBER][FRAME_SIZE];
int Memory_Counter[FRAME_NUMBER];

FILE *backing_store;
char buffer[256];

int TLB_Counter = 0;

void readFromstore(int pageNumber) {
    if(fseek(backing_store,pageNumber*256,SEEK_SET) != 0) {
        printf("Error seeking from backing store\n");
    }

    if(fread(buffer,sizeof(char),256,backing_store) == 0) {
        printf("Error reading from backing store\n");
    }
}

void insertTLB(int pageNumber, int frameNumber) {
    for(int i=0;i<TLB_Counter;i++) {
        if(TLB_Page[i] == pageNumber) {
            break;
        }
    }

    TLB_Page[TLB_Counter] = pageNumber;
    TLB_Frame[TLB_Counter] = frameNumber;

    TLB_Counter = (TLB_Counter + 1) % TLB;
}

int LRU() {
    for(int i=0;i<FRAME_NUMBER;i++) {
        if(Memory_Counter[i] == -1) {
            return i;
        }
    }

    int index = -1;
    int minimum ;
    for(int i=0;i<FRAME_NUMBER;i++) {
        if(i == 0) {
            index = i;
            minimum = Memory_Counter[i];
        }

        if(Memory_Counter[i] < minimum) {
            index = i;
            minimum = Memory_Counter[i];
        }
    }
    return index;
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

    for(int i=0;i<FRAME_NUMBER;i++) {
        Memory_Counter[i] = -1;
    }

    while(fgets(address,INPUT_SIZE,address_file) != NULL) { //reading address
        int logical_address = atoi(address);
        int pageNumber = ((logical_address & 0xFFFF)>>8);
        int offset = (logical_address & 0xFF);
        int frameNumber = -1;
        //printf("%d\n",pageNumber);
        for(int i=0;i<TLB;i++) {
            
            if(TLB_Page[i] == pageNumber) {
                //printf("TLB hit ");
                frameNumber = TLB_Frame[i];
                TLB_Hits++;
            }
        }

        if(frameNumber == -1) { // TLB miss
            //printf("%d ",pageTableValid[pageNumber]);
            if(pageTableValid[pageNumber]) {
                frameNumber = pageTableFrames[pageNumber];
                insertTLB(pageNumber,frameNumber);
            }

            if(frameNumber == -1) { //page fault
                printf("Page fault ");
                readFromstore(pageNumber);

                frameNumber = LRU();
                if (frameNumber == -1) {
                    printf("Error for LRU\n");
                }

                for(int i=0;i<256;i++) {
                    physicalMemory[frameNumber][i] = buffer[i];
                }
                
                for(int i=0;i<PAGE_TABLE;i++) {
                    if(pageTableFrames[i] == frameNumber) {
                        pageTableValid[i] = false;
                    }
                }
                pageTableValid[pageNumber] = true;
                pageTableFrames[pageNumber] = frameNumber;

                int TLB_index = -1;
                for(int i=0;i<TLB;i++) {
                    if(TLB_Frame[i] == frameNumber) {
                        TLB_index = i;
                        break;
                    }
                }

                if(TLB_index == -1) {
                    //printf("Insert TLB ");
                    insertTLB(pageNumber,frameNumber);
                }
                else {
                    for(int i=TLB_index;i>1;i--) {
                        int temp_page;
                        int temp_frame;

                        temp_page = TLB_Page[i];
                        temp_page = TLB_Frame[i];

                        TLB_Page[i] = TLB_Page[i-1];
                        TLB_Frame[i] = TLB_Frame[i-1];

                        TLB_Page[i-1] = temp_page;
                        TLB_Frame[i-1] = temp_frame; 
                    }
                    insertTLB(pageNumber,frameNumber);
                    //TLB_Page[TLB_index] = pageNumber;
                }


                pageFault++;
            }
        }
        //printf("\n");
        //for(int i=0;i<TLB;i++) {
        //    printf("%d %d \n",TLB_Page[i],TLB_Frame[i]);
        //}
        //printf("\n");
        printf("Frame Number: %d ",frameNumber);
        Memory_Counter[frameNumber] = addressCounter;
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


    return 0;
}
