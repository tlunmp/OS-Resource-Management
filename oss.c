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
int resultArray[20];
static int messageQueueId;
int times;
int availableActive = 0;
int randomresources();
int randomInterval();
int randomizeShareablePosition();
int nonTerminated[20];
int num = 0;
int terminatedNumber = 0;
FILE *fp; 
char verbose[] = "on";

//tracker
int numTerminatedDeadLock = 0;
int timesDeadlockRun = 0;	
int trackRequest = 0;
int trackProcessTerminated = 0;
	
void helpMenu();
void signalCall(int signum);
void userProcess();
void initializeQueueArray();
void displayTable();
int ifBlockResources(int fakePid, int result);
void release(int fakePid, int dl);
void checkDeadLockDetection();
void addRequestToAllocated(int fakePid, int results);
void generateInterval(int addInterval);
void nonTerminate();

void generateAvailable();
void generateMaxResource();
void generateLaunch(int addInterval);
void generateShareablePosition();
int generateRequest(int fakePid);
void generateAllocation();
int  randomIntervalLaunch();



int main(int argc, char* argv[]) {
	int c;
	int requestNumbers = 0;
	int maxChildProcess;
	//getopt command for command line
	while((c = getopt (argc,argv, "hv:n:")) != -1) {

		switch(c) {
			case 'h':
				helpMenu();
				return 1;
			case 'v':
				strcpy(verbose, optarg);
				break;
			case 'n': maxChildProcess = atoi(optarg);
				break;
			default:
				fprintf(stderr, "%s: Error: Unknown option -%c\n",argv[0],optopt);
				return -1;	
		}


	}

	fp  = fopen("logfile.txt", "a+");


	maxChildProcess = 100;
	int bufSize = 200;
	int timer = 5;
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
	int totalCount = 0;
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
	

	if(strcmp("on", verbose) == 0) {
		displayTable();
	}

	initializeQueueArray();


	nonTerminate();
	

	//alarm(timer);
	while(totalCount < maxChildProcess || ptr_count > 0){ 					
			
			shmPtr->clockInfo.nanoSeconds += 20000;
			//clock incrementation
			if(shmPtr->clockInfo.nanoSeconds > 1000000000){
				shmPtr->clockInfo.seconds++;
				shmPtr->clockInfo.nanoSeconds -= 1000000000;
			}				
		
		
			if(waitpid(childpid,NULL, WNOHANG)> 0){
				ptr_count--;
			}


			if(ptr_count < 18 && shmPtr->clockInfo.seconds == launchTime.seconds && shmPtr->clockInfo.nanoSeconds > launchTime.nanoSeconds){	
					
					fprintf(stderr,"Generating Log File\n");
/*
					int m;
					for(m=0; m < 20; m++){
						printf("blocked is %d\n",queueArray[m]);
					}
*/
						int l;	
						
						for(l=0; l<18;l++){
							if(nonTerminated[l] == -1){
								terminatedNumber++;					
							} 
						}
							
						if(terminatedNumber == 18){					
							fprintf(fp,"All Process are Terminated.\n");
  	 						fprintf(fp,"Total Request Granted: %d\n", trackRequest);	
  	 						fprintf(fp,"Total Normal Process Terminated: %d\n", trackProcessTerminated );
  	 						fprintf(fp,"Total Killed Process from DeadLock: %d\n", numTerminatedDeadLock);
  	 						fprintf(fp,"Total DeadLock Run: %d\n", timesDeadlockRun);
					
							fprintf(stderr,"All Process are Terminated.\n");
  	 						fprintf(stderr,"Total Request Granted: %d\n", trackRequest);	
  	 						fprintf(stderr,"Total Normal Process Terminated: %d\n", trackProcessTerminated );
  	 						fprintf(stderr,"Total Killed Process from DeadLock: %d\n", numTerminatedDeadLock);
  	 						fprintf(stderr,"Total DeadLock Run: %d\n", timesDeadlockRun);
					//		msgctl(messageQueueId, IPC_RMID, NULL); 
					//		shmdt(shmPtr); //detaches a section of shared memory
    					//		shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory 	
 							return 0;
						//kill(0, SIGTERM);
						} else {
							terminatedNumber = 0;
						}

						if(nonTerminated[num] != -1){
							fakePid = nonTerminated[num];
						} else {	
							
							int s = num;
							for(s=num; s<18;s++){
								if(nonTerminated[s] == -1){
									num++;
								} else {
									break;
								}

							}
							
							fakePid = nonTerminated[num];

							//printf("fakeid is %d",fakePid);
						
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

						totalCount++;
						ptr_count++;
		
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
							

							
							if(strcmp("on", verbose) == 0) {
								fprintf(fp,"Master has detected Process P%d requesting R%d at time %d:%d\n",fakePid, results,shmPtr->clockInfo.seconds,shmPtr->clockInfo.nanoSeconds );
							
							}

							int resultBlocked = ifBlockResources(fakePid,results);

							if(resultBlocked == 0){
								

								
								if(strcmp("on", verbose) == 0) {
									fprintf(fp,"Master blocking P%d requesting R%d at time %d:%d\n",fakePid, results,shmPtr->clockInfo.seconds,shmPtr->clockInfo.nanoSeconds );
								}

								int f, duplicate = 0;
								for(f=0; f< 18; f++){
									if(queueArray[f] == fakePid){
										duplicate++;
									}
								}
								
								if(duplicate == 0){
									queueArray[blockPos] = fakePid;
									resultArray[blockPos] = results;
								} else {
									duplicate = 0;
								}
								blockPos++;					
							} else {
								generateInterval(randomIntervalLaunch());
								addRequestToAllocated(fakePid, results);
								

								if(strcmp("on", verbose) == 0) {
									fprintf(fp,"Master granting P%d  request R%d at time %d:%d\n",fakePid, results, shmPtr->clockInfo.seconds,shmPtr->clockInfo.nanoSeconds);
								}
								if(requestNumbers == 20){
									if(strcmp("on",verbose) == 0){
										displayTable();
									}
									requestNumbers = 0;
								}
								requestNumbers++;
								trackRequest++;
							}
						}	

						if(strcmp(message.mtext, "Terminated") == 0 ){
							trackProcessTerminated++;
							generateInterval(randomIntervalLaunch());
							
							if(strcmp("on", verbose) == 0) {
								fprintf(fp,"Master terminating P%d at %d:%d\n\n",fakePid, shmPtr->clockInfo.seconds,shmPtr->clockInfo.nanoSeconds);
							}
							nonTerminated[fakePid] = -1;

						//	printf("terminated fakePid %d,%d",fakePid, nonTerminated[fakePid]);
							release(fakePid,0);
						}

						if(strcmp(message.mtext, "Release") == 0 ){
		
							generateInterval(randomIntervalLaunch());
							release(fakePid,0);
						}

					
						if(num < 17){			
							num++;	
						} else {
						
							int k,w=0;
							for(k =0; k < 20; k++){
								if(queueArray[k] != -1){
									w++;
								}
							}

							if(w > 0){
								checkDeadLockDetection();
							}

							num = 0;		
							initializeQueueArray();
							blockPos = 0;
						}
		
			
						generateLaunch(randomInterval());	
							

			}
		}

 	 //kill(0, SIGTERM);
	return 0;
}

void nonTerminate(){
	int i;
	for(i = 0; i < 18; i++){
		nonTerminated[i] = i;
	}


}


void displayTable(){
	fprintf(fp,"----------------------------------------------------------------------------------------------------------------------------------------------------\n");
	fprintf(fp,"|                                                               Max Resources                                                                      |\n");	
	fprintf(fp,"----------------------------------------------------------------------------------------------------------------------------------------------------\n");
	fprintf(fp,"|      |");
	int i =0;
	for(i = 0; i < 20; i++) {
		fprintf(fp,"  %2d  |", shmPtr->resources.max[i]);
	}

	fprintf(fp,"\n");
	fprintf(fp,"----------------------------------------------------------------------------------------------------------------------------------------------------\n");
	fprintf(fp,"|                                                            Available Resources                                                                   |\n");	
	fprintf(fp,"----------------------------------------------------------------------------------------------------------------------------------------------------\n");
	fprintf(fp,"|      |");
	for(i = 0; i < 20; i++) {
		fprintf(fp,"  %2d  |", shmPtr->resources.available[i]);

	}

	fprintf(fp,"\n");
	fprintf(fp,"----------------------------------------------------------------------------------------------------------------------------------------------------\n");
	fprintf(fp,"|                                                            Allocated Resources                                                                   |\n");	
	fprintf(fp,"----------------------------------------------------------------------------------------------------------------------------------------------------\n");

	int project = 0;
	int j;
	for(j =0; j <18; j++){
		project = j;

		if(project >=0 && project < 10){
			fprintf(fp,"| %2s%d  |","P", project);

		} else {
			fprintf(fp,"| %s%d  |","P", project);
		}
		for(i = 0; i < 20; i++) {
				fprintf(fp,"  %2d  |", shmPtr->resourceDescriptor[project].allocated[i]);

			}
			fprintf(fp,"\n");
		
	fprintf(fp,"----------------------------------------------------------------------------------------------------------------------------------------------------\n");
	}


}
void helpMenu() {
		printf("---------------------------------------------------------------| Help Menu |--------------------------------------------------------------------------\n");
		printf("-h help menu\n"); 
		printf("-v input(On/Off)              | On: It will print out all the information to the log.  \n"); 
		printf("			      |	Off: It will print only the deadlock detection and how it is resolve\n");
		printf("-n int		              | int for max processor\n"); 
		printf("------------------------------------------------------------------------------------------------------------------------------------------------------\n");
}



void release(int fakePid, int dl){
	int i = 0, j = 0;
	int validationArray[20];
	int returnResult = 0;
	
	for(i=0; i < 20; i++) {
		validationArray[i] = 0;	
	}

	if(strcmp("on", verbose)==0) { 
		fprintf(fp,"Master releasing P%d, Resources are: ",fakePid);
	}

	if(strcmp("off", verbose) == 0 && dl == 1) { 
		fprintf(fp,"Master releasing P%d, Resources are: ",fakePid);
	}



	for(i=0; i < 20; i++) {
		if(shmPtr->resourceDescriptor[fakePid].allocated[i] > 0){		
			validationArray[i] = i;
			
			if(strcmp("on", verbose) == 0) {
				fprintf(fp,"R%d:%d ",i, shmPtr->resourceDescriptor[fakePid].allocated[i]);	
			}
	
			if(strcmp("off", verbose) == 0 && dl == 1) { 
				fprintf(fp,"R%d:%d ",i, shmPtr->resourceDescriptor[fakePid].allocated[i]);		
			}
			j++;
		} else if(shmPtr->resourceDescriptor[fakePid].allocated[i] == 0){
			returnResult++;
		}
	}
	
	

	if(returnResult == 20){
		
		if(strcmp("on", verbose) == 0) {
			fprintf(fp,"None\n");
		}

		if(strcmp("off", verbose) == 0 && dl == 1) { 
		
			fprintf(fp,"None\n");
		}
	} else {
		
		if(strcmp("on", verbose) == 0) {
			fprintf(fp,"\n");
		}

		if(strcmp("off", verbose) == 0 && dl == 1) { 
			fprintf(fp,"\n");
		}

		int i;
		//add to available
		for(i=0; i < 20; i++) {
			
			if(i == shareable[0] || i == shareable[1] || i == shareable[2] || i == shareable[3]){
				shmPtr->resourceDescriptor[fakePid].allocated[i] = 0;
			} else {
				shmPtr->resources.available[i] += shmPtr->resourceDescriptor[fakePid].allocated[i];
				shmPtr->resourceDescriptor[fakePid].allocated[i] = 0;
			}
		
		}
	}
}

void checkDeadLockDetection() {

	timesDeadlockRun++;
	fprintf(fp,"\nCurrent system resources\n");
	fprintf(fp,"Master running deadlock detection at time %d:%d\n",shmPtr->clockInfo.seconds, shmPtr->clockInfo.nanoSeconds);
	int i = 0;
	int manyBlock = 0;	

	for(i =0; i < 20; i++){
		if(queueArray[i] != -1){
			manyBlock++;
		}
	}
	

	fprintf(fp,"	Process ");

	for(i =0; i < manyBlock; i++){
		fprintf(fp, "P%d, ", queueArray[i]);

	}	
	fprintf(fp,"deadlocked\n");
	fprintf(fp,"	Attempting to resolve deadlock...\n");
/*
	for(i =0; i < manyBlock; i++){
		printf( "%d\n", resultArray[i]);
	}	

*/		
	for(i =0; i < manyBlock; i++){	
		if(shmPtr->resources.available[resultArray[i]] <= shmPtr->resourceDescriptor[queueArray[i]].request[resultArray[i]] ){
			fprintf(fp,"	Killing process P%d\n", queueArray[i]);
			fprintf(fp,"		");
			release(queueArray[i],1);		
			
			if(i+1 < manyBlock){
				fprintf(fp,"	Master running deadlock detection after P%d killed\n",queueArray[i]);
				fprintf(fp,"	Processes ");
				numTerminatedDeadLock++;
				int m;
				for(m=i+1; m <manyBlock; m++){
					fprintf(fp,"P%d, ",queueArray[m]);	
				}
			
				fprintf(fp,"deadlocked\n");
			}
		} else {
			addRequestToAllocated(queueArray[i], resultArray[i]);
			fprintf(fp,"	Master granting P%d request R%d at time %d:%d\n",queueArray[i], resultArray[i], shmPtr->clockInfo.seconds,shmPtr->clockInfo.nanoSeconds);
			//displayTable();
	
			//printf("%d:%d\n",shmPtr->resources.available[resultArray[i]],shmPtr->resourceDescriptor[queueArray[i]].request[resultArray[i]]);
		}
	}	
	fprintf(fp,"System is no longer in deadlock\n");
	fprintf(fp,"\n");
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

}


int  generateRequest(int fakePid) {
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
	times = rand() % (25000000 + 1 - 1) + 1;
	return times;
}


int  randomInterval() {
	int times = 0; 
	times = rand() % (50000000 + 1 - 1) + 1;
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

   fprintf(stderr,"Total Request Granted: %d\n", trackRequest);	
   fprintf(stderr,"Total Normal Process Terminated: %d\n", trackProcessTerminated );
   fprintf(stderr,"Total Killed Process from DeadLock: %d\n", numTerminatedDeadLock);
   fprintf(stderr,"Total DeadLock Run: %d\n", timesDeadlockRun);
	
    //clean up program before exit (via interrupt signal)
    shmdt(shmPtr); //detaches a section of shared memory
    shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory
   
  	 kill(0, SIGTERM);
      exit(EXIT_SUCCESS);
 }
