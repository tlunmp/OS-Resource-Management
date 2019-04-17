#include "resources.h"


#define MESSAGEKEY 3000

typedef struct message {
    long myType;
    char mtext[512];
} Message;


void signalCall(int signum);

int shmid; 
SharedMemory * shmPtr;
int messageQueueId;
Clock maxTimeBetweenNewProcs;
Clock userReal;
int times;
int maxforks = 1;


int randomInterval();
void signalCall(int signum);
void generateLaunch(int addInterval);


int main(int argc, char* argv[]) {

	int bufSize = 200;
	int timer = 20;
	char errorMessage[bufSize];
	Message message;


	pid_t chilpid;
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
	alarm(timer);
	
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

	srand(NULL);
 	time_t t;
	 srand((unsigned) time(&t));
	
	int i;
	for(i=0; i < 3; i++) {	
		int times = randomInterval();
		printf("time is %d\n",times);

		generateLaunch(times);

		printf("launch time is %d:%d\n", shmPtr->clockInfo.seconds, shmPtr->clockInfo.nanoSeconds);
	}

	
/*
	while(totalCount < maxChildProcess && totalCount < lines ){ 					

			shmPtr->clockInfo.nanoSeconds += 20000;
			//clock incrementation
			if(shmPtr->clockInfo.nanoSeconds > 1000000000){
				shmPtr->clockInfo.seconds++;
				shmPtr->clockInfo.nanoSeconds -= 1000000000;
			}				
		

		
			if(waitpid(0,NULL, WNOHANG)> 0)
				ptr_count--;

			if(shmPtr->clockInfo.seconds == maxTimeBetweenNewProcs.seconds && shmPtr->clockInfo.nanoSeconds > maxTimeBetweenNewProcs.nanoSeconds){	
				char buffer1[100];
				sprintf(buffer1, "%d", totalCount);
				childpid=fork();

							ptr_count++;
				totalCount++;
		
				if(childpid < 0) {
					perror("Fork failed");
				} else if(childpid == 0) {		
					execl("./user", "user", buffer1,(char*)0);
					snprintf(errorMessage, sizeof(errorMessage), "%s: Error: ", argv[0]);
	    	 			perror(errorMessage);		
					exit(0);
				} else {
					
				}


				message.myType = 1;	
				strcpy(message.mText,"20");
	
				if(msgsnd(messageQueueId, &message,sizeof(message)+1,0) == -1) {
					perror("msgsnd");
					exit(1);
				}

	

				if (msgrcv(messageQueueId, &message,sizeof(message)+1,2,0) == -1) {
					perror("msgrcv");

				}	

				maxTimeBetweenNewProcs.nanoSeconds += 200000;
		}
	
	}


*/	
	shmdt(shmPtr); //detaches a section of shared memory
    	shmctl(shmid, IPC_RMID, NULL);  // deallocate the memory 

	return 0;
}

void generateLaunch(int addInterval) {
	shmPtr->clockInfo.nanoSeconds += addInterval;
	
	if(shmPtr->clockInfo.nanoSeconds > 1000000000){
		shmPtr->clockInfo.seconds++;
		shmPtr->clockInfo.nanoSeconds -= 1000000000;
	}				
	
} 

int  randomInterval() {
	int times = 0; 
	times = rand() % (500000000 + 1 - 1) + 1;
	return times;
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


