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
#include <time.h>

int shmid; 
int child_id;
int chance[100];
int chancePos =0;
SharedMemory* shmPtr;


typedef struct message {
    long myType;
    char mtext[512];
} Message;

static int messageQueueId;
int terminate = 0;

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


 	 
	shmPtr = shmat(shmid, NULL, 0);
		//srand(NULL);
 		//time_t t;
	srand(getpid());
		
/*	
	int i=0;
	for(i=0; i <100; i++){
		chance[i] = rand() % (100 + 1 - 1) + 1;
		printf("chanes are %d\n",chance[i]);
	}*/

	while(1) {
	
		if (msgrcv(messageQueueId, &message,sizeof(message)+1,1,0) == -1) {
			perror("msgrcv");
		}

		int chance = rand() % (100 + 1 - 1) + 1;
		
//		printf("message recieve is %s\n", message.mtext); 
		printf("\n chance are %d\n",chance);	
		
		//request
		if(chance > 1 && chance < 53) {
			strcpy(message.mtext,"Request");

		//release
		} else if(chance > 52 && chance < 73) {
			strcpy(message.mtext,"Release");

		//terminated
		} else if(chance > 72 && chance  < 101) {	
			strcpy(message.mtext,"Terminated");
			terminate = 1;
		}
	
		message.myType = 2;	
			
	//	strcpy(message.mtext,"Release");
		if(msgsnd(messageQueueId, &message,sizeof(message)+1,0) == -1) {
			perror("msgsnd");
			exit(1);
		} 

			exit(0);
	
  	 	//kill(0, SIGTERM);

	}	
	return 0;
}
