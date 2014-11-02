
#include "ThreadFunctions.h"

PageInfo* GetPage(MemManager* mem) {
    
    srand((unsigned int)time(0));
    PageInfo* page = NULL;
    while ((page = mem->GetFreePage()) == NULL) {
        unsigned int rnd = 0;
        while((rnd = rand()%10) < 1);
        usleep(rnd);
    }
    return page;
}

inline bool TagChar(char c) {
    return (c == '<' || c == '>' || c == ':' || (c >= '0' && c <='9'));
}

bool SetChunkInf(ChunkInfo *inf, string *temp){
    
    if (temp->size() < 5) return false;
    
    unsigned long int del = 0;
    del = temp->find(':');
    if (del == string::npos) return false;
    
    if (temp->find_last_of(':') != del) return false;
    
    inf->startByte = atoi(&(*temp)[1])-1;
    inf->fileID = atoi(&(*temp)[del+1]);
    return true;
}

void Write2Mem(string *buffer, ChunkInfo *inf) {
    
    MemManager* mem = MemManager::GetInstance();
    PageInfo* page = NULL;
    
    string::iterator it;
    for (it = buffer->begin(); it != buffer->end(); it++) {
        
        if (page == NULL) {
            page = GetPage(mem);
            page->SetPageWrite(inf);
        }
        
        if (!page->Write((*it))) {
            page = NULL;
            it--;
        }
        else inf->startByte++;
    }
    if (page != NULL) {
        page->CloseWritePage();
    }
    
}

void* ReadFile(void* arg) {
    
    int filenum = *(int*)arg;
    
    string fileName = REC_DIR+itoa(filenum)+FILE_EXT;
    ifstream file;
    file.open(fileName.c_str(), ios_base::in);
    file >> noskipws;
    
    string buffer;
    string temp;
    
    char curr = '\0';
    
    ChunkInfo currInf, newInf;
    buffer.clear();
    
    while (file >> curr) {
        bool newSeg = false;
        bool append = true;
        if (curr == '<') {
            temp.clear();
            temp+=curr;
            while (file >> curr){
                if (!TagChar(curr)) break;
                if (curr == '<') {
                    buffer += temp;
                    temp.clear();
                    temp += curr;
                    continue;
                }
                if (curr == '>') {
                    temp += curr;
                    append = false;
                    newSeg = SetChunkInf(&newInf, &temp);
                    
                    break;
                }
                temp += curr;
            }
        }
        
        if (!newSeg) {
            buffer += temp;
            if (append)
                buffer += curr;
            temp.clear();
        }
        else {
            if (currInf.fileID != 0) {
                Write2Mem(&buffer, &currInf);
            }
            temp.clear();
            buffer.clear();
            currInf.Set(newInf);
        }
    }
    if (buffer.size()){
        if (currInf.fileID != 0)
            Write2Mem(&buffer, &currInf);
    }
    file.close();
    
    pthread_exit(NULL);
    
}

void* WriteFile(void* arg) {
    
    int filenum = *(int*)arg;
    int semIdx = filenum-1;
    
    MemManager* mem = MemManager::GetInstance();
    PageInfo* page = NULL;

    string fileName = OUT_DIR+itoa(filenum)+FILE_EXT;
    fstream file;
    
    while (!allDone) {
        
        if((page = mem->FindCorrespondingPage(filenum)) == NULL) continue;
        
        sem_wait(semas[semIdx]);
        
        unsigned long int offs = page->fileOffs;
        string* temp = page->Read();
        string data = *temp;
        
        file.open(fileName.c_str(), ios_base::in | ios_base::out);
        file.seekp(offs); file << data; file.flush();
        file.close();
        
        delete temp;
        sem_post(semas[semIdx]);
    }    
    
    pthread_exit(NULL);
}