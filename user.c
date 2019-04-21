#include "resources.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>	
#include <semaphore.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <fcntl.h>

int shmid; 
int child_id;

SharedMemory* shmPtr;


typedef struct message {
    long myType;
    char mtext[512];
} Message;

static int messageQueueId;


int main(int argc, char* argv[]) {

	Message message;	

     	if ((shmid = shmget(SHMKEY, sizeof(SharedMemory), 0600)) < 0) {
            perror("Error: shmget");
            exit(errno);
     	}
   
    	 if ((messageQueueId = msgget(MESSAGEKEY, 0644)) == -1) {
            perror("Error: msgget");
            exit(errno);
      	}

	srand(NULL);
 	time_t t;
	 srand((unsigned) time(&t));
	

 	 
	shmPtr = shmat(shmid, NULL, 0);

	printf("%d, user launced time is %d:%d\n",getpid(),shmPtr->clockInfo.seconds, shmPtr->clockInfo.nanoSeconds);				
	while(1) {
	

		if (msgrcv(messageQueueId, &message,sizeof(message)+1,1,0) == -1) {
			perror("msgrcv");
		}
		
		printf("message recieve is %s\n", message.mtext);

		int chance = rand() % (100 + 1 - 1) + 1;

		printf("\n Chance is %d\n",chance);
		if(chance > 1 || chance < 33) {



		} 

		if(chance >1 || chance < 33) {



		} 


	}	
//			printf("entering critical section\n");


	return 0;
}
