
#pragma once

#include <iostream>
#include "MemManager.h"

using namespace std;

#define PAGE_SIZE 256

enum PageState {
    PROG_READ,
    PROG_WRITE,
    FULL,
    FREE
};

class ChunkInfo {
public:
    int fileID;
    unsigned long int startByte;
    void Set(ChunkInfo arg) {
        fileID = arg.fileID;
        startByte = arg.startByte;
    }
};

class PageInfo {
public:
    
    PageState state;
    void* pageaddr;
    int fid;
    int size;
    unsigned long int offset, fileOffs;
    
    PageInfo(void* addr);
    bool Write(char c);
    string* Read();
    
    void SetPageWrite(const ChunkInfo *inf);
    void CloseWritePage();
    
    PageState GetState() { return state;}
    void SetState(PageState s) { state = s;}
    
    int GetFile() { return fid;}
    void SetFile(int fileID) { fid = fileID;}
    
    void ResetOffset() { offset = 0;}
   
    
};
