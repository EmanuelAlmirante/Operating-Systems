#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

#include <buffer.h>

info* buffer;
int buff_size;

int init_buffer(int buff) {
	int i;

	buff_size=buff;

	buffer=(info*)malloc(sizeof(info)*buff_size);

	for(i = 0; i < buff_size; i++){
		buffer[i].ptr_request = (request*) malloc(sizeof(request)); //Allocates memory for the requests.
		buffer[i].ptr_answer = (answer*) malloc(sizeof(answer)); //Allocates memory for the requests.
		buffer[i].ptr_semaphores = (semaphores*) malloc(sizeof(semaphores)); //Allocates memory for the requests.
		sem_init(&(buffer[i].ptr_semaphores->sem_client), 0, 0); //Initializes the client's semaphore.
		sem_init(&(buffer[i].ptr_semaphores->sem_server), 0, 0); //Initializes the server's semaphore.
	}

	if(!buffer) {
		return -1;
	}
	
	return 0;
}
