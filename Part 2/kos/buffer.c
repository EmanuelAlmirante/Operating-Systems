#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

#include <buffer.h>

info* buffer;
int buff_size;

int init_buffer(int buff) {
	int i;

	buff_size = buff;

	buffer = (info*)malloc(sizeof(info)*buff_size);

	for(i = 0; i < buff_size; i++) {
		buffer[i].ptr_request = (request*) malloc(sizeof(request)); //Allocates memory for the requests.
		buffer[i].ptr_answer = (answer*) malloc(sizeof(answer)); //Allocates memory for the answers.
		buffer[i].ptr_semaphore = (semaphore*) malloc(sizeof(semaphore)); //Allocates memory for the semaphores.
	}

	if(!buffer) {
		return -1;
	}
	
	return 0;
}
