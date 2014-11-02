
#include "PageInfo.h"

PageInfo::PageInfo(void* addr){
    state = FREE;
    fid = 0;
    offset = 0;
    pageaddr = addr;
}

bool PageInfo::Write(char c) {
    if (state == PROG_WRITE) {
        ((char*)pageaddr)[offset] = c;
        offset++;
        size++;
        if (offset == PAGE_SIZE/sizeof(char)) {
            state = FULL;
            offset = 0;
            sem_post(MemManager::ready_chunks);
        }
        return true;
    }
    else
        return false;
}
string* PageInfo::Read() {
    if (state == PROG_READ) {
        string* t = new string((char*)pageaddr, size);
        state = FREE;
        size = 0;
        offset = 0;
        sem_post(MemManager::mem_space);
        return t;
    }
    return NULL;
}

void PageInfo::SetPageWrite(const ChunkInfo *inf) {
    state = PROG_WRITE;
    fid = inf->fileID;
    offset = 0;
    size = 0;
    fileOffs = inf->startByte;
}

void PageInfo::CloseWritePage() {
    state = FULL;
    offset = 0;
    sem_post(MemManager::ready_chunks);
}