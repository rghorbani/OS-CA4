
#include "MemManager.h"

MemManager::MemManager() {
    
    shmid = shmget(IPC_PRIVATE, PAGE_SIZE*PAGES, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid == -1) {
        cerr << "[ERR] Couldn't allocate a shared memory segment\n";
        exit(-2);
    }
    shmaddr = NULL;
    shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void*)-1) {
        cerr << "[ERR] Shared memory attachment failed.\n";
        perror("[shmat]");
        exit(-3);
    }
    
    shmctl(shmid, IPC_RMID, (struct shmid_ds*)NULL);
    
    for (int i = 0; i < PAGES; i++) {
        pageInfs[i] = new PageInfo((char*)shmaddr+(i*PAGE_SIZE));
    }
    
    mem_space = sem_open(ALLOC_SEM,  O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, PAGES);
    if (mem_space == SEM_FAILED) {
        perror("SEM_ERR[SPACE]");
        exit(-4);
    }
    ready_chunks = sem_open(READ_SEM,  O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, 0);
    if (ready_chunks == SEM_FAILED) {
        perror("SEM_ERR[READY]");
        exit(-4);
    }
    
    display = sem_open(DISP_MUX,  O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, 1);
    if (display == SEM_FAILED) {
        perror("SEM_ERR[DISP]");
        exit(-4);
    }
    pageAllocMux = sem_open(ALLOC_MUX,  O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, 1);
    if (pageAllocMux == SEM_FAILED) {
        perror("SEM_ERR[ALLOC]");
        exit(-4);
    }
    pageReadMux = sem_open(READ_MUX,  O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, 1);
    if (pageReadMux == SEM_FAILED) {
        perror("SEM_ERR[READ]");
        exit(-4);
    }
    manager = this;
}

PageInfo* MemManager::GetFreePage(){
    sem_wait(mem_space);
    for (int i = 0; i < PAGES; i++) {
        sem_wait(pageAllocMux);
        if ((*pageInfs[i]).GetState() == FREE) {
            (*pageInfs[i]).SetState(PROG_WRITE);
            sem_post(pageAllocMux);
            sem_wait(display);
            cout << "\n[WRITE] Page " << i <<
            " has been allocated as a free page for writing operation.\n";
            sem_post(display);

            return pageInfs[i];
        }
        else sem_post(pageAllocMux);
    }
    return NULL;
}

PageInfo* MemManager::FindCorrespondingPage(int fileID) {
    if(sem_trywait(ready_chunks) != 0) {
        usleep(1);
        if (sem_trywait(ready_chunks)) {
            return NULL;
        }
    }
    for (int i = 0; i < PAGES; i++) {
        sem_wait(pageReadMux);
        if ((*pageInfs[i]).GetState() == FULL) {
            if ((*pageInfs[i]).GetFile() == fileID) {
                pageInfs[i]->SetState(PROG_READ);
                unsigned long int offs = pageInfs[i]->fileOffs;
                sem_post(pageReadMux);
                sem_wait(display);
                cout << "\n[READ] Page " << i <<
                " has been set for reading operation.\n[READ] It contains a piece of file " << fileID <<
                " which starts from byte " << offs << " of the file.\n";
                sem_post(display);
                return pageInfs[i];
            }
            else {
                sem_post(pageReadMux);
            }
        }
        else {
            sem_post(pageReadMux);
        }
    }
    sem_post(ready_chunks);
    return NULL;
}
MemManager::~MemManager() {
    sem_unlink(DISP_MUX);
    sem_close(display);
    sem_unlink(ALLOC_MUX);
    sem_close(pageAllocMux);
    sem_unlink(READ_MUX);
    sem_close(pageReadMux);
    sem_unlink(ALLOC_SEM);
    sem_close(mem_space);
    sem_unlink(READ_SEM);
    sem_close(ready_chunks);
    
    shmdt(shmaddr);
    for (int i = 0; i < PAGES; i++) {
        delete pageInfs[i];
    }
}

void MemManager::WaitForFlush() {
    for (int i = 0; i < PAGES; i++) {
        while (pageInfs[i]->GetState() != FREE);
    }
}

MemManager* MemManager::manager = NULL;
sem_t* MemManager::mem_space = NULL;
sem_t* MemManager::ready_chunks = NULL;
sem_t* MemManager::display = NULL;
sem_t* MemManager::pageAllocMux = NULL;
sem_t* MemManager::pageReadMux = NULL;