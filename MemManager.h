
#pragma once

#include <iostream>
#include <semaphore.h>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "PageInfo.h"


using namespace std;

#define PAGES 100
#define ALLOC_SEM "ALLC_SEMA4"
#define READ_SEM "READ_SEMA4"
#define DISP_MUX "DISP_MUTex"
#define ALLOC_MUX "ALLC_MUTex"
#define READ_MUX "READ_MUTEex"

extern volatile bool allDone;
class PageInfo;
class MemManager {
    PageInfo* pageInfs[PAGES];
    int shmid;
    void* shmaddr;
    static MemManager* manager;
    MemManager();
    
public:
    static sem_t* display;
    static sem_t* pageAllocMux;
    static sem_t* pageReadMux;
    static sem_t* mem_space;
    static sem_t* ready_chunks;
    static MemManager* GetInstance() {
        if(manager != NULL) return manager;
        manager = new MemManager();
        return manager;
    };
    PageInfo* GetFreePage();
    PageInfo* FindCorrespondingPage(int fileID);
    void WaitForFlush();
    ~MemManager();
};
