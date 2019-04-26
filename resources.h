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
#define MESSAGEKEY 3000
#define MAXPROCESSES 18
#define RESOURCESAMT 20

typedef struct clock {
	int seconds;
	int nanoSeconds;
} Clock;

struct mesgQ {
    long mType;
    char mText[100];
} messenger;

typedef struct {
	int max[RESOURCESAMT];
	int available[RESOURCESAMT];
} Resource;

typedef struct {
       int allocated[RESOURCESAMT];
       int request[RESOURCESAMT];
	int isTerminated;
} ResourceDescriptor;

typedef struct {
	int fakePid;
	int deadLockResources[20];
} DeadLock;

typedef struct shared_memory_object {
    ResourceDescriptor resourceDescriptor[MAXPROCESSES + 1];
    DeadLock deadLock[MAXPROCESSES + 1];
    Resource resources;
    Clock clockInfo;
} SharedMemory; 
