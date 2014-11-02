
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <csignal>
#include "lib.h"
#include "ThreadFunctions.h"

using namespace std;

#define WR_RATE 2

#define FILE_NAME(id, ext) itoa(id)+ext

int numOfReaders;
int numOfWriters;
pthread_t *writers;
pthread_t *readers;
sem_t** semas;
MemManager *mem;
volatile bool allDone;

void KillAndExit(int sem_i) {
    for (int j = 0; j < sem_i; j++) {
        string fileName = FILE_NAME(j+1, FILE_EXT);
        sem_unlink(fileName.c_str());
    }
    delete mem;
}

void signal_handler(int signum) {
    
    for (int i = 0; i < numOfWriters; i++) {
        pthread_cancel(writers[i]);
    }
    for (int i = 0; i < numOfReaders; i++) {
        pthread_cancel(readers[i]);
    }
    for (int j = 0; j < numOfWriters/WR_RATE; j++) {
        string fileName = FILE_NAME(j+1, FILE_EXT);
        sem_unlink(fileName.c_str());
    }
    delete mem;
    
    exit(signum);
}

void RunAllThreads() {
    for (int i = 0; i < numOfReaders; i++) {
        pthread_join(readers[i], NULL);
    }
    mem->WaitForFlush();
    allDone = true;
    for (int i = 0; i < numOfWriters; i++) {
        pthread_join(writers[i], NULL);
    }
    for (int i = 0; i < numOfWriters/WR_RATE; i++) {
        string name = FILE_NAME(i+1, FILE_EXT);
        sem_unlink(name.c_str());
    }
}

int main(int argc, const char * argv[]) {
    
    try {
        
        signal(SIGINT, signal_handler);
        
        if (argc != 3) {
            cerr << "[ERR] Usage: <Num of Recoverd Files> <Num of Initial File>\n";
            return -1;
        }
        
        numOfReaders = atoi(argv[1]);
        numOfWriters = WR_RATE*atoi(argv[2]);
        if(numOfReaders < 1 || numOfWriters < 2) {
            cerr << "[ERR] Invlalid Arguments.\n";
            return -2;
        }
        
        int status = 0;
        
        if((status = mkdir(OUT_DIR, S_IRWXO | S_IRWXG | S_IRWXU)) != 0) {
            if (errno != EEXIST){
                perror("[MKDIR]");
                return -3;
            }
        }
        
        if((writers = (pthread_t*)calloc(numOfWriters, sizeof(pthread_t))) == NULL) {
            perror("[WRITERS_MEM]");
            return -4;
        }
        if((readers = (pthread_t*)calloc(numOfReaders, sizeof(pthread_t))) == NULL) {
            perror("[READERS_MEM]");
            return -5;
        }
        if((semas = (sem_t**)calloc(numOfWriters/WR_RATE, sizeof(sem_t*))) == NULL) {
            perror("[SEMAS_MEM]");
        }
        mem = MemManager::GetInstance();
        if (mem == NULL) {
            cerr << "[MEM_MGR]: Unable to set memory manager\n";
        }
        allDone = false;
        
        int fileNumber = 1;
        for (int i = 0; i < numOfWriters/WR_RATE; i++) {
            
            string fileName = FILE_NAME(fileNumber, FILE_EXT);
            
            if((semas[i] = sem_open(fileName.c_str(),  O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, 1)) == SEM_FAILED) {
                cerr << "[SEMAS_" << fileName << "]: ";
                perror("");
                KillAndExit(fileNumber);
                return -6;
            }
            fileName = OUT_DIR+fileName;
            ofstream out(fileName.c_str(), ios_base::trunc);
            if (out.is_open()) {
                out.close();
                fileNumber++;
            }
            else {
                KillAndExit(fileNumber+1);
                cerr << "[FOPEN_ERR]:" << fileName << endl;
                return -7;
            }
        }
    
        fileNumber = 1;
        for (int i = 0; i < numOfReaders; i++) {
            if((status = pthread_create(&readers[i], NULL, ReadFile, (void*)new int(fileNumber))) != 0){
                KillAndExit(numOfWriters/2);
                cerr << "[READ_THREAD_ERR]:" << i+1 << endl;
                return -8;
            }
            fileNumber++;
        }
        
        fileNumber = 1;
        
        for (int i = 0; i < numOfWriters; i += 2) {
            if((status = pthread_create(&writers[i], NULL, WriteFile, new int(fileNumber))) != 0){
                KillAndExit(numOfWriters/2);
                cerr << "[WRITE_THREAD_ERR]:" << " file: " << fileNumber << ", thread" << (i%2)+1 << endl;
                return -9;
            }
            if((status = pthread_create(&writers[i+1], NULL, WriteFile, new int(fileNumber))) != 0){
                KillAndExit(numOfWriters/2);
                cerr << "[WRITE_THREAD_ERR]:" << " file: " << fileNumber << ", thread" << ((i+1)%2)+1 << endl;
                return -9;
            }
            fileNumber++;
        }
        
        //Wait for all threads to finish
        RunAllThreads();
        delete mem;
        return 0;
    }
    catch(...) {
        KillAndExit(numOfWriters/WR_RATE);
        return -1;
    }
}

