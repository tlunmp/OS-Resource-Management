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
int availableActive = 0;
int deadLock[20];

int randomresources();
int randomInterval();
int randomizeShareablePosition();

void signalCall(int signum);

void userProcess();
void initializeQueueArray();
int checkIfDeadLockUponRequest(int fakePid);	
void ifUpdateAvailableResources(int result, int fakePid);
void updateAllocationToAvailable(int fakePid);
int  checkIfDeadLockUponAlloc(int fakePid);	

void generateAvailable();
void generateMaxResource();
void generateLaunch(int addInterval);
void generateShareablePosition();
void generateRequest(int fakePid);
void generateAllocation(int fakePid);

int main(int argc, char* argv[]) {

	int bufSize = 200;
	int timer = 10;
	char errorMessage[bufSize];
	Message message;


	int ptr_count = 0;


	//sigalarm error
	if (signal(SIGALRM, signalCall) == SIG_ERR) {
            snprintf(errorMessage, sizeof(errorMessage), "%s: Error: user: signal(): SIGALRM\n", argv[0]);
	     perror(errorMessage);	
         	exit(errno);
     	}
	

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
	int maxChildProcess = 4;
	int status = 0;
	int blockPos = 0;


	generateLaunch(randomInterval());

	//generate the total resources
	generateMaxResource();

	//randomly generate shareable position
	generateShareablePosition();

	//initializing the available resources
	generateAvailable();


	initializeQueueArray();

	//alarm for 2 real life second
	alarm(2);

	int deadLockPos = 0;
	
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

	
				generateAllocation(fakePid);
				

				int results = checkIfDeadLockUponAlloc(fakePid);	
				
				if(results == 1){
					updateAllocationToAvailable(fakePid);
				} else {
					
					printf("%d, deadLock\n", fakePid);
					deadLock[deadLockPos] = fakePid;			
					deadLockPos++;
					break;

				}


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
					
					printf("P%d is requesting\n", fakePid);
					int result = checkIfDeadLockUponRequest(fakePid);
			//		printf("result is %d", result);

					ifUpdateAvailableResources(result,fakePid);
			//		int i;
			//		for(i = 0; i < 20; i++) {	
			//		printf("%d request is %d: %d\n",fakePid,i,shmPtr->resourceDescriptor[fakePid].request[i]);
			//		}
		
				}	

				if(strcmp(message.mtext, "Terminated") == 0 ){
				
				}

				
				if(fakePid < 19){
					fakePid++;	
				} else {
					fakePid = 0;
				}


				generateLaunch(randomInterval());
			}
	
	}

	//msgctl(messageQueueId, IPC_RMID, NULL); 
	//shmdt(shmPtr); //detaches a section of shared memory
    	//shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory 
		
  	// kill(0, SIGTERM);
	return 0;
}


//update the available by using the max - allocation
void updateAllocationToAvailable(int fakePid){
	int i;	
	for(i=0; i < 20; i++) {
	
		for(i=0; i < 20; i++) {
			
				if(i == shareable[0] || i == shareable[1] || i == shareable[2] || i == shareable[3]){
					
					shmPtr->resources.available[i] = shmPtr->resources.max[i];
					printf("position %d, aloc available is %d\n", i, shmPtr->resources.available[i]);
				} else {
					shmPtr->resources.available[i] = shmPtr->resources.available[i] - shmPtr->resourceDescriptor[fakePid].allocated[i];	
					printf("position %d, aloc available is %d\n", i, shmPtr->resources.available[i]);
				}
			}
		


	}

}


void ifUpdateAvailableResources(int result, int fakePid) {
	if(result == 1) {

		if(availableActive == 0){
		int i;
			for(i=0; i < 20; i++) {
			
				if(i == shareable[0] || i == shareable[1] || i == shareable[2] || i == shareable[3]){
					
					shmPtr->resources.available[i] = shmPtr->resources.max[i];
					printf("position %d, print available is %d\n", i, shmPtr->resources.available[i]);
				} else {
					shmPtr->resources.available[i] = shmPtr->resources.available[i] - shmPtr->resourceDescriptor[fakePid].request[i];	
					printf("position %d, print available is %d\n", i, shmPtr->resources.available[i]);
				}
			}
		
		} 
	}
}


void generateShareablePosition(){
	int i;
	for(i=0; i < 4; i++){
		shareable[i] = randomizeShareablePosition();
		printf("shareable is %d\n",shareable[i]);

	}
}




//return 1 if not deadlock return 0 if it is deadlock
int  checkIfDeadLockUponAlloc(int fakePid){	
	
	//check shmPtr->resources max[i] and shmPtr->resourceDescriptor[fakePid].request[i]	
	int i,j;
	int validationArray[20];

	for(i = 0; i < 20; i++){
		if(shmPtr->resources.available[i] > shmPtr->resourceDescriptor[fakePid].allocated[i] || shmPtr->resources.available[i] == shmPtr->resourceDescriptor[fakePid].allocated[i]){
					validationArray[i] = 1;
			//		printf("position %d: array = %d,", i, validationArray[i]);
			//		printf("greater %d,%d\n",shmPtr->resources.available[i], shmPtr->resourceDescriptor[fakePid].request[i]);
			} else {
					validationArray[i] = 0;
			//		printf("position %d: array = %d,", i, validationArray[i]);
			//			printf("%d,%d\n",shmPtr->resources.available[i], shmPtr->resourceDescriptor[fakePid].request[i]);
			}
		//	printf("%d = %d\n", shmPtr->resources.max[i], shmPtr->resourceDescriptor[fakePid].request[i]);
	}

	
		for(j = 0; j < 20; j++){
			if(validationArray[j] == 0){
				return 0;
			} 
		}
	return 1;
}





//return 1 if not deadlock return 0 if it is deadlock
int  checkIfDeadLockUponRequest(int fakePid){	
	
	//check shmPtr->resources max[i] and shmPtr->resourceDescriptor[fakePid].request[i]	
	int i,j;
	int validationArray[20];

	for(i = 0; i < 20; i++){
		if(shmPtr->resources.available[i] > shmPtr->resourceDescriptor[fakePid].request[i] || shmPtr->resources.available[i] == shmPtr->resourceDescriptor[fakePid].request[i]){
					validationArray[i] = 1;
			//		printf("position %d: array = %d,", i, validationArray[i]);
			//		printf("greater %d,%d\n",shmPtr->resources.available[i], shmPtr->resourceDescriptor[fakePid].request[i]);
			} else {
					validationArray[i] = 0;
			//		printf("position %d: array = %d,", i, validationArray[i]);
			//			printf("%d,%d\n",shmPtr->resources.available[i], shmPtr->resourceDescriptor[fakePid].request[i]);
			}
		//	printf("%d = %d\n", shmPtr->resources.max[i], shmPtr->resourceDescriptor[fakePid].request[i]);
	}

	
		for(j = 0; j < 20; j++){
			if(validationArray[j] == 0){
				return 0;
			} 
		}
	return 1;
}




void generateAllocation(int fakePid){
	int i = 0;

	for(i = 0; i < 20; i++) {	
		shmPtr->resourceDescriptor[fakePid].allocated[i] = rand() % (2 + 0 - 0) + 0;
		//printf("%d, allocated rss %d\n",i, shmPtr->resourceDescriptor[fakePid].allocated[i]);
	}

}

void initializeQueueArray(){
	int i = 0;
	for(i = 0; i <20; i++){
		queueArray[i] = -1;
	}	
}


void generateRequest(int fakePid) {
	int i = 0;
	for(i = 0; i < 20; i++) {	
		shmPtr->resourceDescriptor[fakePid].request[i] = rand() % (2 + 0 - 0) + 0;
		printf("P%d: request rss %d\n", fakePid, shmPtr->resourceDescriptor[fakePid].request[i]);
	}
}

void generateAvailable(){
	int i =0 ;
	for(i=0; i <20; i++){
		shmPtr->resources.available[i] = shmPtr->resources.max[i];
	//	printf("%d, available is %d:%d\n", i, shmPtr->resources.max[i], shmPtr->resources.available[i]);
	}
}

void generateMaxResource(){
	int i =0 ;
	for(i=0; i <20; i++){
		shmPtr->resources.max[i] = randomResources();
		//printf("max rss %d\n",shmPtr->resources.max[i]);
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
  	 kill(0, SIGTERM);
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
    //clean up program before exit (via interrupt signal)
    shmdt(shmPtr); //detaches a section of shared memory
    shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory
   
      exit(EXIT_SUCCESS);
 }


