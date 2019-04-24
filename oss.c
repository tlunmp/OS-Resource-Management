#include "resources.h"

typedef struct message {
    long myType;
    char mtext[512];
} Message;


void signalCall(int signum);

int isNext = 0;
int shmid; 
SharedMemory* shmPtr;
Clock launchTime;

int shareable[4];
int queueArray[20];
int resultArray[20];
static int messageQueueId;
int times;
int availableActive = 0;
int randomresources();
int randomInterval();
int randomizeShareablePosition();


void signalCall(int signum);
void userProcess();
void initializeQueueArray();
void displayTable();
int ifBlockResources(int fakePid, int result);
void release(int fakePid);
void checkDeadLockDetection();
void addRequestToAllocated(int fakePid, int results);
void generateInterval(int addInterval);


void generateAvailable();
void generateMaxResource();
void generateLaunch(int addInterval);
void generateShareablePosition();
int generateRequest(int fakePid);
void generateAllocation();
int  randomIntervalLaunch();

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
	int maxChildProcess = 30;
	int status = 0;
	int blockPos = 0;


	generateLaunch(randomInterval());

	//generate the total resources
	generateMaxResource();

	//randomly generate shareable position
	generateShareablePosition();

	//initializing the available resources
	generateAvailable();
	
	//generate empty allocation	
	generateAllocation();

	//display the table
	displayTable();

	initializeQueueArray();

	//alarm for 2 real life second
	alarm(2);



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
					

/*
					int m;
					for(m=0; m < 20; m++){
						printf("blocked is %d\n",queueArray[m]);
					}
*/	

							
						totalCount++;
		
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
	
		//				printf("oss recieve %s\n", message.mtext);	

						if(strcmp(message.mtext, "Request") == 0 ){
	
							int results = generateRequest(fakePid);
							printf("Master has detected Process P%d requesting R%d at time %d:%d\n",fakePid, results,shmPtr->clockInfo.seconds,shmPtr->clockInfo.nanoSeconds );
							int resultBlocked = ifBlockResources(fakePid,results);

							if(resultBlocked == 0){
								printf("Master blocking P%d requesting R%d at time %d:%d\n",fakePid, results,shmPtr->clockInfo.seconds,shmPtr->clockInfo.nanoSeconds );
								
								queueArray[blockPos] = fakePid;
								resultArray[blockPos] = results;
								/*
								shmPtr->deadLock[blockPos].fakePid = fakePid;
								
								int i;
								for(i=0; i <20;i++){
									shmPtr->deadLock[blockPos].deadLockResources[i] = shmPtr->resourceDescriptor[fakePid].allocated[i];
								}*/
								blockPos++;					
							} else {
								generateInterval(randomIntervalLaunch);
								addRequestToAllocated(fakePid, results);
								printf("Master granting P%d  request R%d at time %d:%d\n",fakePid, results, shmPtr->clockInfo.seconds,shmPtr->clockInfo.nanoSeconds);
								displayTable();
							}
						}	

						if(strcmp(message.mtext, "Terminated") == 0 ){
							printf("terminated P%d\n",fakePid);	
							//printf("Master terminating  P%d  Releasing  R%d ",fakePid, results, shmPtr->clockInfo.seconds,shmPtr->clockInfo.nanoSeconds);
						}

						if(strcmp(message.mtext, "Release") == 0 ){
		
							release(fakePid);
							displayTable();
							//printf("Master terminating  P%d  Releasing  R%d ",fakePid, results, shmPtr->clockInfo.seconds,shmPtr->clockInfo.nanoSeconds);
						}

					
						if(fakePid < 17){			
							fakePid++;	
						} else {
							checkDeadLockDetection();
							isNext == 0;
							fakePid = 0;		
							initializeQueueArray();
							blockPos = 0;
						}
		
			
					

					generateLaunch(randomInterval());	
			}
		}

//	msgctl(messageQueueId, IPC_RMID, NULL); 
	//shmdt(shmPtr); //detaches a section of shared memory
    	//shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory 
	
 //	kill(0, SIGTERM);
	return 0;
}




void displayTable(){
	printf("----------------------------------------------------------------------------------------------------------------------------------------------------\n");
	printf("|                                                               Max Resources                                                                      |\n");	
	printf("----------------------------------------------------------------------------------------------------------------------------------------------------\n");
	printf("|      |");
	int i =0;
	for(i = 0; i < 20; i++) {
		printf("  %2d  |", shmPtr->resources.max[i]);
	}

	printf("\n");
	printf("----------------------------------------------------------------------------------------------------------------------------------------------------\n");
	printf("|                                                            Available Resources                                                                   |\n");	
	printf("----------------------------------------------------------------------------------------------------------------------------------------------------\n");
	printf("|      |");
	for(i = 0; i < 20; i++) {
		printf("  %2d  |", shmPtr->resources.available[i]);

	}

	printf("\n");
	printf("----------------------------------------------------------------------------------------------------------------------------------------------------\n");
	printf("|                                                            Allocated Resources                                                                   |\n");	
	printf("----------------------------------------------------------------------------------------------------------------------------------------------------\n");

	int project = 0;
	int j;
	for(j =0; j <18; j++){
		project = j;

		if(project >=0 && project < 10){
			printf("| %2s%d  |","P", project);

		} else {
			printf("| %s%d  |","P", project);
		}
		for(i = 0; i < 20; i++) {
				printf("  %2d  |", shmPtr->resourceDescriptor[project].allocated[i]);

			}
			printf("\n");
		
	printf("----------------------------------------------------------------------------------------------------------------------------------------------------\n");
	}


}

void release(int fakePid){
	int i = 0, j = 0;
	int validationArray[20];
	int returnResult = 0;

	for(i=0; i < 20; i++) {
		validationArray[i] = 0;	
	}

//	shmPtr->resourceDescriptor[fakePid].allocated[5] = 5;
//	shmPtr->resourceDescriptor[fakePid].allocated[6] = 5;
	
	printf("Master releasing P%d, Resources are: ",fakePid);
	
	for(i=0; i < 20; i++) {
		if(shmPtr->resourceDescriptor[fakePid].allocated[i] > 0){		
			validationArray[j] = i;	
			printf("R%d:%d ",i, shmPtr->resourceDescriptor[fakePid].allocated[i]);	
			j++;
		} else if(shmPtr->resourceDescriptor[fakePid].allocated[i] == 0){
			returnResult++;
		}
	}


	if(returnResult == 20){
		printf("None\n",fakePid);
	} else {
		printf("\n");
		int i;
		//add to available
		for(i=0; i < 20; i++) {
			shmPtr->resources.available[i] += shmPtr->resourceDescriptor[fakePid].allocated[i];
		}
	}
}

void checkDeadLockDetection() {
	printf("Current system resources\n");
	printf("Master running deadlock detection at time %d:%d\n",shmPtr->clockInfo.seconds, shmPtr->clockInfo.nanoSeconds);
	int i = 0;
	int manyBlock = 0;	
	int deadLockArray[20];

	for(i =0; i < 20; i++){
		if(queueArray[i] != -1){
			manyBlock++;
		}
	}

	int bufSize = 100;
	char buffer[bufSize];
	

	printf("	Process ");

	for(i =0; i < manyBlock; i++){
		printf( "P%d, ", queueArray[i]);

	}	
	int j = 0;
	printf("deadlocked\n");
	printf("	Attempting to resolve deadlock...\n");
/*
	for(i =0; i < manyBlock; i++){
		printf( "%d\n", resultArray[i]);
	}	

*/		
	for(i =0; i < manyBlock; i++){	
		if(shmPtr->resources.available[resultArray[i]] <= shmPtr->resourceDescriptor[queueArray[i]].request[resultArray[i]] ){
			printf("	Killing process P%d\n", queueArray[i]);
			printf("		");
			release(queueArray[i]);	
			printf("	Master running deadlock detection after P%d killed\n",queueArray[i]);
			printf("	Processes ");
			int m;
			for(m=i+1; m <manyBlock; m++){
				printf("P%d, ",queueArray[m]);	
			}
			
			printf("deadlocked\n");
		} else {
			addRequestToAllocated(queueArray[i], resultArray[i]);
			printf("	Master granting P%d request R%d at time %d:%d\n",queueArray[i], resultArray[i], shmPtr->clockInfo.seconds,shmPtr->clockInfo.nanoSeconds);
			displayTable();
	
			//printf("%d:%d\n",shmPtr->resources.available[resultArray[i]],shmPtr->resourceDescriptor[queueArray[i]].request[resultArray[i]]);
		}
	}	
	printf("System is no longer in deadlock\n");
}


//when requesting
int ifBlockResources(int fakePid, int result) {
	if(shmPtr->resources.available[result] >= shmPtr->resourceDescriptor[fakePid].request[result] ){
	//	printf("%d:%d\n", shmPtr->resources.available[result], shmPtr->resourceDescriptor[fakePid].request[result]);
		return 1;
	} else {
		return 0;
	}
}


void addRequestToAllocated(int fakePid, int results) {
	

	if(results == shareable[0] || results == shareable[1] || results == shareable[2] || results == shareable[3]){
		shmPtr->resources.available[results] = shmPtr->resources.max[results];
	} else {
		shmPtr->resources.available[results] = shmPtr->resources.available[results] - shmPtr->resourceDescriptor[fakePid].request[results];
	}
	shmPtr->resourceDescriptor[fakePid].allocated[results] = shmPtr->resourceDescriptor[fakePid].request[results];
}




void generateAllocation(){
	int i = 0;
	int j = 0;
	for(j=0; j < 18; j++) {
		for(i = 0; i < 20; i++) {	
			shmPtr->resourceDescriptor[j].allocated[i] = 0;
			//printf("P%d: %d, allocated rss %d\n", fakePid, i, shmPtr->resourceDescriptor[fakePid].allocated[i]);
		}

	}
}

void initializeQueueArray(){
	int i = 0;
	for(i = 0; i <20; i++){
		queueArray[i] = -1;
	}	
	

/*
	for(j=0; j < 18; j++){
		shmPtr->deadLock[j].fakePid = -1;
		for(i = 0; i <20; i++){
			shmPtr->deadLock[j].deadLockRequest[i] = -1;
		}	
	}*/
}


int  generateRequest(int fakePid) {
	int i = 0;
	int resourcesLoc = rand() % (19 + 0 - 0) + 0;
	shmPtr->resourceDescriptor[fakePid].request[resourcesLoc] = rand() % (10 + 1 - 1) + 1;
//	printf("P%d: request rss at R%d,  %d\n", fakePid, resourcesLoc,  shmPtr->resourceDescriptor[fakePid].request[resourcesLoc]);
	return resourcesLoc;
}


void generateShareablePosition(){
	int i;
	for(i=0; i < 4; i++){
		shareable[i] = randomizeShareablePosition();
		//printf("shareable is %d\n",shareable[i]);

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

void generateInterval(int addInterval){
	shmPtr->clockInfo.nanoSeconds += addInterval;
	
	if(shmPtr->clockInfo.nanoSeconds > 1000000000){
		shmPtr->clockInfo.seconds++;
		shmPtr->clockInfo.nanoSeconds -= 1000000000;
	}
}

void generateLaunch(int addInterval) {
	launchTime.nanoSeconds += addInterval;
	
	if(launchTime.nanoSeconds > 1000000000){
		launchTime.seconds++;
		launchTime.nanoSeconds -= 1000000000;
	}				
	
} 

int  randomIntervalLaunch() {
	int times = 0; 
	times = rand() % (250000000 + 1 - 1) + 1;
	return times;
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


