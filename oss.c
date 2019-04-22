#include "resources.h"

typedef struct message {
    long myType;
    char mtext[512];
} Message;


void signalCall(int signum);

int shmid; 
SharedMemory* shmPtr;
Clock launchTime;

int shareable[4];
int queueArray[20];
static int messageQueueId;
int times;

int randomresources();
int randomInterval();
int randomizeShareablePosition();


void generateShareablePosition();
void signalCall(int signum);
void generateLaunch(int addInterval);
void generateMaxResource();
void userProcess();
void initializeQueueArray();
void generateRequest(int fakePid);
void checkIfDeadLockUponRequest(int fakePid);	

int main(int argc, char* argv[]) {

	int bufSize = 200;
	int timer = 20;
	char errorMessage[bufSize];
	Message message;


	int ptr_count = 0;

	
	//signal error
	 if (signal(SIGINT, signalCall) == SIG_ERR) {
        	snprintf(errorMessage, sizeof(errorMessage), "%s: Error: user: signal(): SIGINT\n", argv[0]);
		perror(errorMessage);	
        	exit(errno);
  	  }
	
	//sigalarm error
	if (signal(SIGALRM, signalCall) == SIG_ERR) {
            snprintf(errorMessage, sizeof(errorMessage), "%s: Error: user: signal(): SIGALRM\n", argv[0]);
	     perror(errorMessage);	
         	exit(errno);
     	}
	
	//alarm for 2 real life second
	alarm(2);
	
	if ((shmid = shmget(SHMKEY, sizeof(SharedMemory), IPC_CREAT | 0600)) < 0) {
        	perror("Error: shmget");
        	exit(errno);
	}
  
 
	if ((messageQueueId = msgget(MESSAGEKEY, IPC_CREAT | 0644)) == -1) {
        	perror("Error: mssget");
       		 exit(errno);
    	}
  
	 shmPtr = shmat(shmid, NULL, 0);
  	 shmPtr->clockInfo.seconds = 0; 
   	 shmPtr->clockInfo.nanoSeconds = 0;  

	pid_t childpid;

	int fakePid = 0;
	shmPtr->clockInfo.nanoSeconds = 0;
	shmPtr->clockInfo.seconds = 0;

	srand(NULL);
 	time_t t;
	srand((unsigned) time(&t));
	int processCount = 0;
	int totalCount = 0;
	int maxChildProcess = 3;
	int status = 0;
	int blockPos = 0;


	generateLaunch(randomInterval());

	//generate the total resources
	generateMaxResource();

	initializeQueueArray();

	while(totalCount < maxChildProcess){ 					
			shmPtr->clockInfo.nanoSeconds += 20000;
			//clock incrementation
			if(shmPtr->clockInfo.nanoSeconds > 1000000000){
				shmPtr->clockInfo.seconds++;
				shmPtr->clockInfo.nanoSeconds -= 1000000000;
			}				
		
		
			if(waitpid(0,NULL, WNOHANG)> 0)
				ptr_count--;

			if(shmPtr->clockInfo.seconds == launchTime.seconds && shmPtr->clockInfo.nanoSeconds > launchTime.nanoSeconds){	
									
				totalCount++;
				printf("totalCount is %d\n",totalCount)	;			

				message.myType = 1;
				char buffer1[100];
				sprintf(buffer1, "%d", fakePid);
				strcpy(message.mtext,buffer1);	
			
				if(msgsnd(messageQueueId, &message,sizeof(message)+1,0) == -1) {
					perror("msgsnd");
					exit(1);
				}


				// char buffer1[100];
				// sprintf(buffer1, "%d", totalCount);
				childpid=fork();

		
				if(childpid < 0) {
					perror("Fork failed");
				} else if(childpid == 0) {		
					execl("./user", "user",NULL);
					snprintf(errorMessage, sizeof(errorMessage), "%s: Error: ", argv[0]);
	    	 			perror(errorMessage);		
					exit(0);
				} else {
		
				}
		
				if (msgrcv(messageQueueId, &message,sizeof(message)+1,2,0) == -1) {
					perror("msgrcv");

				}	
	
				printf("oss recieve %s\n", message.mtext);	

				if(strcmp(message.mtext, "Block") == 0 ){
					queueArray[blockPos] = fakePid;
					blockPos++;
				}	

				if(strcmp(message.mtext, "Request") == 0 ){
					generateRequest(fakePid);
					
					checkIfDeadLockUponRequest(fakePid);
					int i;
					for(i = 0; i < 20; i++) {	
						printf("%d request is %d: %d\n",fakePid,i,shmPtr->resourceDescriptor[fakePid].request[i]);
					}
				}	

				
				if(strcmp(message.mtext, "Terminated") == 0 ){
				
				}

				fakePid++;	
				generateLaunch(randomInterval());
			}
	
	}

	//msgctl(messageQueueId, IPC_RMID, NULL); 
	//shmdt(shmPtr); //detaches a section of shared memory
    	//shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory 
	return 0;
}

void generateShareablePosition(){
	int i;
	for(i=0; i < 4; i++){
		shareable[i] = randomizeShareablePosition();
	}
}


void checkIfDeadLockUponRequest(int fakePid){	
	
	//check shmPtr->resources max[i] and shmPtr->resourceDescriptor[fakePid].request[i]	
	int i;
	for(i = 0; i < 20; i++){

		printf("%d = %d\n", shmPtr->resources.max[i], shmPtr->resourceDescriptor[fakePid].request[i]);
	}

}

void generateRequest(int fakePid) {
	int i = 0;
	int times;

	for(i = 0; i < 20; i++) {	
		shmPtr->resourceDescriptor[fakePid].request[i] = rand() % (3 + 1 - 1) + 1;
	}
}

void initializeQueueArray(){
	int i = 0;
	for(i = 0; i <20; i++){
		queueArray[i] = -1;
	}	
}


void userProcess() {
	



}

void generateMaxResource(){
	int i =0 ;
	for(i=0; i <20; i++){
		shmPtr->resources.max[i] = randomResources();
	}
}

void generateLaunch(int addInterval) {
	launchTime.nanoSeconds += addInterval;
	
	if(launchTime.nanoSeconds > 1000000000){
		launchTime.seconds++;
		launchTime.nanoSeconds -= 1000000000;
	}				
	
} 

int  randomInterval() {
	int times = 0; 
	times = rand() % (500000000 + 1 - 1) + 1;
	return times;
}

int randomResources() {
	int resources = 0;
	resources = rand() % (10 + 1 - 1) + 1;
	return resources;
}


int randomizeShareablePosition() {
	int shareable = 0;
	shareable = rand() % (19 + 0 - 0) + 0;
	return shareable;
}


//signal calls
void signalCall(int signum)
{
    int status;
  //  kill(0, SIGTERM);
    if (signum == SIGINT)
        printf("\nSIGINT received by main\n");
    else
        printf("\nSIGALRM received by main\n");
 
    while(wait(&status) > 0) {
        if (WIFEXITED(status))  /* process exited normally */
                printf("User process exited with value %d\n", WEXITSTATUS(status));
        else if (WIFSIGNALED(status))   /* child exited on a signal */
                printf("User process exited due to signal %d\n", WTERMSIG(status));
        else if (WIFSTOPPED(status))    /* child was stopped */
                printf("User process was stopped by signal %d\n", WIFSTOPPED(status));
    }
    kill(0, SIGTERM);
    //clean up program before exit (via interrupt signal)
    shmdt(shmPtr); //detaches a section of shared memory
    shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory
   
      exit(EXIT_SUCCESS);
 }


