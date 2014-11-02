
#pragma once

#include "lib.h"
#include "PageInfo.h"
#include <fstream>
#include <string>
#include <unistd.h>
#include <pthread.h>

using namespace std;

#define OUT_DIR "corrected files/"
#define REC_DIR "recovered files/"
#define FILE_EXT ".txt"

extern volatile bool allDone;
extern sem_t** semas;

void* ReadFile(void* arg);
void* WriteFile(void* arg);