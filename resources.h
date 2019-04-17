#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>	
#include <semaphore.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <time.h>

#define SHMKEY 9784

#define MAX_USER_PROCESSES 18

typedef struct clock {
	int seconds;
	int nanoSeconds;
} Clock;

typedef struct {
    unsigned int seconds;
    unsigned int nanoseconds;
} systemClock_t;

struct mesgQ {
    long mType;
    char mText[100];
} messenger;

typedef struct {
       int processWorking[18];
       int available[20];
       int max[18][20];
       int allocated[18][20];
       int request[18][20];
} ResourceDescriptor;

typedef struct shared_memory_object {
    ResourceDescriptor resourceDescriptor[MAX_USER_PROCESSES + 1];
    Clock clockInfo;
} SharedMemory; 


