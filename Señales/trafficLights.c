/*
 ============================================================================
 Name        : EjerciciosSeñales.c
 Author      : José Cisneros
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

// Traffic Light Duration
#define SLEEP_FOR 5
// Functions
void ctrlZHandler(int s);
void ctrlCHandler(int s);
void returnToPrevState();
void lightChanged(int s);
void change1(int s);
void change2(int s);
void change3(int s);
void change4(int s);

//	Global variables
int * shmaddr;
int ctrlC = 0;
int ctrlZ = 0;

int main(void) {
	//	Shared Memory
	key_t shm_key = 232323;
	const int shm_size = 12 * sizeof(int); // (pid,state,previous state) (0 = Red, 1 = Yellow, 2 = Green)
	int shm_id;
	shm_id = shmget (shm_key, shm_size, IPC_CREAT | S_IRUSR | S_IWUSR);
	shmaddr = (int*) shmat (shm_id, 0, 0);

// Traffic Lights
	// Traffic Light 1
	int light1 = fork();
		if (light1 < 0) // Error
			printf("ERROR: Couldn't create traffic light 1.\n");
		else if(light1 == 0){ // If child
			*shmaddr = (int) getpid(); // Set Light 1's PID
			*(shmaddr + 1) = 2; // Set Light 1's state to green
			*(shmaddr + 2) = 0; // Set Light 1's previous state to red.
			printf("Traffic Light 1 created. \n");
			while(1){
				if (signal(SIGUSR1, change1) == SIG_ERR)  // Signal from another light.
					printf("ERROR: Signal manager not found.\n");
				if(*(shmaddr + 1) == 2){
					sleep(SLEEP_FOR);
					kill(*(shmaddr+3),SIGUSR1);
				}
			}
		}
	// Traffic Light 2
	int light2 = fork();
		if (light2 < 0) // Error
			printf("ERROR: Couldn't create traffic light 2.\n");
		else if(light2 == 0){ // If child
			*(shmaddr + 3) = (int) getpid(); // Set Light 2's PID
			*(shmaddr + 4) = 0; // Set Light 2's state to red.
			*(shmaddr + 5) = 0; // Set Light 2's previous state to red.
			printf("Traffic Light 2 created. \n");
			while(1){
				if (signal(SIGUSR1, change2) == SIG_ERR)  // Signal from another light.
					printf("ERROR: Signal manager not found.\n");
				if(*(shmaddr + 4) == 2){
					sleep(SLEEP_FOR);
					kill(*(shmaddr+6),SIGUSR1);
				}
			}
		}
	// Traffic Light 3
	int light3 = fork();
	if (light3 < 0) // Error
		printf("ERROR: Couldn't create traffic light 3.\n");
	else if(light3 == 0){ // If child
		*(shmaddr + 6) = (int) getpid(); // Set Light 3's PID.
		*(shmaddr + 7) = 0; // Set Light 3's state to red.
		*(shmaddr + 8) = 0; // Set Light 3's previous state to red.
		printf("Traffic Light 3 created. \n");
		while(1){
			if (signal(SIGUSR1, change3) == SIG_ERR)  // Signal from another light.
				printf("ERROR: Signal manager not found.\n");
			if(*(shmaddr + 7) == 2){
				sleep(SLEEP_FOR);
				kill(*(shmaddr+9),SIGUSR1);
			}
		}
	}
	// Traffic Light 4
	int light4 = fork();
	if (light4 < 0) // Error
		printf("ERROR: Couldn't create traffic light 4.\n");
	else if(light4 == 0){ // If child
		*(shmaddr + 9) = (int) getpid(); // Set Light 4's PID.
		*(shmaddr + 10) = 0; // Set Light 4's state to red.
		*(shmaddr + 11) = 0; // Set Light 4's previous state to red.
		printf("Traffic Light 4 created. \n");
		while(1){
			if (signal(SIGUSR1, change4) == SIG_ERR)  // Signal from another light.
				printf("ERROR: Signal manager not found.\n");
			if(*(shmaddr + 10) == 2){
				sleep(SLEEP_FOR);
				kill(*shmaddr,SIGUSR1);
			}
		}
	}
	//	Ctrl + C (All Yellow)
	if(signal(SIGINT, ctrlCHandler) == SIG_ERR)
		printf("ERROR: Signal manager not found. \n");	
	// Ctrl + Z (All Red)
	if(signal(SIGTSTP, ctrlZHandler) == SIG_ERR)
		printf("ERROR: Signal manager not found. \n");
	// Light Changed
	if(signal(SIGUSR1, lightChanged) == SIG_ERR) // If a light changed, print all lights' state.
		printf("ERROR: Signal manager not found. \n");

	//Wait for Children
	waitpid(light1,0,0);
	waitpid(light2,0,0);
	waitpid(light3,0,0);
	waitpid(light4,0,0);

	/* Detach the shared memory segment. */
		 shmdt (shmaddr);
	/* Deallocate the shared memory segment.*/
		 shmctl (shm_id, IPC_RMID, 0);
	return EXIT_SUCCESS;
}

void lightChanged(int s){
	printf("--------------------------\n");
	int i;
	for(i = 0; i < 4; ++i){
		int state = *(shmaddr + ((3*i)+1));
		switch(state){
		case 0: printf("Traffic Light %d is Red.\n",i+1);
			break;
		case 1: printf("Traffic Light %d is Yellow.\n",i+1);
			break;
		case 2: printf("Traffic Light %d is Green.\n",i+1);
			break;
		}
	}
}
void ctrlZHandler(int s){
	if(ctrlZ == 0){
		ctrlZ = 1;
	printf("Changing all lights to red...\n");
	int i;
	for(i = 0; i < 4; ++i){
		*(shmaddr + ((3*i)+2)) = *(shmaddr + ((3*i)+1)); // Save current state as previous state
		*(shmaddr + ((3*i)+1)) = 0; // Turn to red
	}
	lightChanged(0);
	}
	else{
		ctrlZ = 0;
		returnToPrevState();
	}
}
void ctrlCHandler(int s){
	if(ctrlC == 0){
	ctrlC = 1;
	printf("Changing all lights to yellow...\n");
	int i;
	for(i = 0; i < 4; ++i){
		*(shmaddr + ((3*i)+2)) = *(shmaddr + ((3*i)+1)); // Save current state as previous state
		*(shmaddr + ((3*i)+1)) = 1; // Turn to yellow
	}
	lightChanged(0);
	}
	else{
		ctrlC = 0;
		returnToPrevState();
	}
}

void returnToPrevState(){
	printf("Changing all lights to their previous state...\n");
	int i,pid;
	for(i = 0; i < 4; ++i){
		*(shmaddr + ((3*i)+1)) = *(shmaddr + ((3*i)+2));
		*(shmaddr + ((3*i)+2)) = 0;
		if(*(shmaddr + ((3*i)+1)) == 2)
			pid = *(shmaddr + (3*i));
	}
	kill(pid,SIGUSR1);
	lightChanged(0);
}

void change1(int s){
	*(shmaddr + 10) = 0; // Change Light4's state to red.
	*(shmaddr + 11) = 2; // Change Light4's previous state to green.
	*(shmaddr + 1) = 2; // Change Light1's state to green.
	*(shmaddr + 2) = 0; // Change Light1's previous state to red.
	kill(getppid(),SIGUSR1);
}
void change2(int s){
	*(shmaddr + 1) = 0; // Change Light1's state to red.
	*(shmaddr + 2) = 2; // Change Light1's previous state to green.
	*(shmaddr + 4) = 2; // Change Light2's state to green.
	*(shmaddr + 5) = 0; // Change Light2's previous state to red.
	kill(getppid(),SIGUSR1);
}
void change3(int s){
	*(shmaddr + 4) = 0; // Change Light2's state to red.
	*(shmaddr + 5) = 2; // Change Light2's previous state to green.
	*(shmaddr + 7) = 2; // Change Light3's state to green.
	*(shmaddr + 8) = 0; // Change Light3's previous state to red.
	kill(getppid(),SIGUSR1);
}
void change4(int s){
	*(shmaddr + 7) = 0; // Change Light3's state to red.
	*(shmaddr + 8) = 2; // Change Light3's previous state to green.
	*(shmaddr + 10) = 2; // Change Light4's state to green.
	*(shmaddr + 11) = 0; // Change Light4's previous state to red.
	kill(getppid(),SIGUSR1);
}
