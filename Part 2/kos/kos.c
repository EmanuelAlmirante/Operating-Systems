#include <kos_client.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <server.h>
#include <buffer.h>
#include <hash_t.h>
#include <list.h>
#include <mutexes.h>
#include <files.h>

#define KV_SIZE 20

extern list_t*** hash_t;
extern info *buffer;
extern int buff_size;
extern pthread_mutex_t* mutex_buf_cli;

sem_t can_produce; //Semaphore that controls the production.
sem_t can_consume; //Semaphore that controls the consumption.

int pos_buf_cli=0; //Position of the buffer that each client thread will access.

int counter; //Number of keys in the shard requested by kos_getAllKeys;

int kos_init(int num_server_threads, int buf_size, int num_shards) {

	int success = 0; //Variable that saves the "success" of each function.

	success = init_hash_t(num_shards); 	//Initializes the hash table.

	if(success != 0) {
		return -1;
	}

	success = init_buffer(buf_size); //Initializes the buffer.

	if(success != 0) {
		return -1;
	}

	success = init_mutex(num_shards); //Initializes the mutexes.

	if(success != 0) {
		return -1;
	}

	success = init_files(num_shards); //Initializes the files.

	if(success != 0) {
		return -1;
	}

	success = imports_file(num_shards);	//Imports the files, if any exist.

	if(success != 0) {
		return -1;
	}

	success = init_servers(num_shards, num_server_threads);	//Initializes the servers.

	if(success != 0) {
		return -1;
	}

	sem_init(&can_consume, 0, 0); //Initializes the semaphore.
	sem_init(&can_produce, 0, buff_size); //Initializes the semaphore with the size of the buffer (number of simultaneous requests).

	return 0;
}

char* kos_get(int clientid, int shardId, char* key) {
	request* new_request;
	answer* new_answer;
	semaphore* new_semaphore;
	char* ret_value=NULL;

	new_request = (request*) malloc(sizeof(request)); //Allocates memory for the new request.
	strcpy(new_request->key, key); //Puts the key in the request.
	memset(new_request->value, 0, sizeof(new_request->value)); //"Cleans" the string 'value' of the new request.
	new_request->shardId = shardId; //Puts the id of the shard in the request.
	new_request->command = 1; //Command 1: 'kos_get'.

	new_answer=(answer*) malloc(sizeof(answer)); //Allocates memory for the new answer.
	memset(new_answer->value, 0, sizeof(new_answer->value)); //"Cleans" the string 'value' of the new answer.
	new_answer->ret = 0; //Puts the value 0 in the return value.

	new_semaphore=(semaphore*) malloc(sizeof(semaphore)); //Allocates memory for the new semaphore.
	sem_init(&(new_semaphore->sem_answer), 0, 0); //Initializes the new semaphore.

	sem_wait(&can_produce); //Blocks itself waiting that it can put a new request in the buffer.
	pthread_mutex_lock(&mutex_buf_cli[pos_buf_cli]);

	buffer[pos_buf_cli].ptr_request = new_request; //Puts the new request in the buffer.
	buffer[pos_buf_cli].ptr_answer = new_answer; //Puts the new answer in the buffer.
	buffer[pos_buf_cli].ptr_semaphore = new_semaphore; //Puts the new semaphore in the buffer.
	pos_buf_cli = (pos_buf_cli + 1) % buff_size; //Indicates the next position of the buffer to be accessed.

	pthread_mutex_unlock(&mutex_buf_cli[pos_buf_cli]);
	sem_post(&can_consume); //Signals the semaphore of consumption about the requests in the buffer.
	sem_wait(&(new_semaphore->sem_answer)); //Blocks itself waiting for the request being processed.

	if(new_answer->ret == 0) {
		ret_value = (char*)malloc(KV_SIZE*sizeof(char)); //Allocates memory for the return value.
		strcpy(ret_value, new_answer->value); //Copies the value 'value' to the return variable.
		free(new_request); //Releases the allocated memory for the new request.
		free(new_answer); //Releases the allocated memory for the new answer.
		free(new_semaphore); //Releases the allocated memory for the new semaphore.
		return ret_value;
	}

	free(new_request);
	free(new_answer);
	free(new_semaphore);

	return NULL;
}

char* kos_put(int clientid, int shardId, char* key, char* value) {
	request* new_request;
	answer* new_answer;
	semaphore* new_semaphore;
	char* ret_value=NULL;

	new_request = (request*) malloc(sizeof(request)); //Allocates memory for the new request.
	strcpy(new_request->key, key); //Puts the key in the request.
	strcpy(new_request->value, value); //Puts the value in the request.
	new_request->shardId = shardId; //Puts the id of the shard in the request.
	new_request->command = 2; //Command 2: 'kos_put'.

	new_answer=(answer*) malloc(sizeof(answer)); //Allocates memory for the new answer.
	memset(new_answer->value, 0, sizeof(new_answer->value)); //"Cleans" the string 'value' of the new answer.
	new_answer->ret=0; //Puts the value 0 in the return value.

	new_semaphore=(semaphore*) malloc(sizeof(semaphore)); //Allocates memory for the new semaphore.
	sem_init(&(new_semaphore->sem_answer), 0, 0);

	sem_wait(&can_produce); //Blocks itself waiting that it can put a new request in the buffer.
	pthread_mutex_lock(&mutex_buf_cli[pos_buf_cli]);

	buffer[pos_buf_cli].ptr_request = new_request; //Puts the new request in the buffer.
	buffer[pos_buf_cli].ptr_answer = new_answer; //Puts the new answer in the buffer.
	buffer[pos_buf_cli].ptr_semaphore = new_semaphore; //Puts the new semaphore in the buffer.
	pos_buf_cli = (pos_buf_cli + 1) % buff_size; //Indicates the next position of the buffer to be accessed.

	pthread_mutex_unlock(&mutex_buf_cli[pos_buf_cli]);
	sem_post(&can_consume); //Server can process request.
	sem_wait(&(new_semaphore->sem_answer)); //Blocks itself waiting for the server to process the answer.

	if(new_answer->ret == 0) {
		ret_value = (char*) malloc(KV_SIZE*sizeof(char)); //Allocates memory for the return value.
		strcpy(ret_value, new_answer->value); //Copies the value 'value' to the return value.
		free(new_request); //Releases the allocated memory for the new request.
		free(new_answer); //Releases the allocated memory for the new answer.
		free(new_semaphore); //Releases the allocated memory for the new semaphore.
		return ret_value;
	}

	free(new_request);
	free(new_answer);
	free(new_semaphore);

	return NULL;
}

char* kos_remove(int clientid, int shardId, char* key) {
	request* new_request;
	answer* new_answer;
	semaphore* new_semaphore;
	char* ret_value = NULL;

	new_request = (request*) malloc(sizeof(request)); //Allocates memory for the new request.
	strcpy(new_request->key, key); //Puts the key in the request.
	memset(new_request->value, 0, sizeof(new_request->value)); //"Cleans" the string 'value' of the new request.
	new_request->shardId = shardId; //Puts the id of the shard in the request.
	new_request->command = 3; //Command 3: 'kos_remove'.

	new_answer=(answer*) malloc(sizeof(answer)); //Allocates memory for the new answer.
	memset(new_answer->value, 0, sizeof(new_answer->value)); //"Cleans" the string 'value' of the new answer.
	new_answer->ret = 0; //Puts the value 0 in the return value.

	new_semaphore = (semaphore*) malloc(sizeof(semaphore));
	sem_init(&(new_semaphore->sem_answer), 0, 0);

	sem_wait(&can_produce); //Blocks itself waiting that it can put a new request in the buffer.
	pthread_mutex_lock(&mutex_buf_cli[pos_buf_cli]);

	buffer[pos_buf_cli].ptr_request = new_request; //Puts the new request in the buffer.
	buffer[pos_buf_cli].ptr_answer = new_answer; //Puts the new answer in the buffer.
	buffer[pos_buf_cli].ptr_semaphore = new_semaphore; //Puts the new semaphore in the buffer.
	pos_buf_cli = (pos_buf_cli + 1) % buff_size; //Indicates the next position of the buffer to be accessed.

	pthread_mutex_unlock(&mutex_buf_cli[pos_buf_cli]);
	sem_post(&can_consume); //Server can process request.
	sem_wait(&(new_semaphore->sem_answer)); //Blocks itself waiting for the server to process the answer.

	if(new_answer->ret==0){
		ret_value=(char*)malloc(KV_SIZE*sizeof(char)); //Allocates memory for the return value.
		strcpy(ret_value, new_answer->value); //Copies the value 'value' to the return value.
		free(new_request); //Releases the allocated memory for the new request.
		free(new_answer); //Releases the allocated memory for the new answer.
		free(new_semaphore);  //Releases the allocated memory for the new semaphore.
		return ret_value;
	}

	free(new_request);
	free(new_answer);
	free(new_semaphore);

	return NULL;
}

KV_t* kos_getAllKeys(int clientid, int shardId, int* dim) {
	request* new_request;
	answer* new_answer;
	semaphore* new_semaphore;
	KV_t* ret_value = NULL;

	new_request = (request*) malloc(sizeof(request)); //Allocates memory for the new request.
	memset(new_request->key, 0, sizeof(new_request->key)); //"Cleans" the string 'key' of the new request.
	memset(new_request->value, 0, sizeof(new_request->value)); //"Cleans" the string 'value' of the new request.
	new_request->shardId = shardId; //Puts the id of the shard in the request.
	new_request->command = 4; //Command 4: 'kos_getAllKeys'.

	new_answer = (answer*) malloc(sizeof(answer)); //Allocates memory for the new answer.
	memset(new_answer->value, 0, sizeof(new_answer->value)); //"Cleans" the string 'value' of the new answer.
	new_answer->vec = NULL;
	new_answer->ret = 0; //Puts the value 0 in the return value.

	new_semaphore = (semaphore*) malloc(sizeof(semaphore));	//Allocates memory for the new semaphore.
	sem_init(&(new_semaphore->sem_answer), 0, 0);

	sem_wait(&can_produce); //Blocks itself waiting that it can put a new request in the buffer.
	pthread_mutex_lock(&mutex_buf_cli[pos_buf_cli]);

	buffer[pos_buf_cli].ptr_request = new_request; //Puts the new request in the buffer.
	buffer[pos_buf_cli].ptr_answer = new_answer; //Puts the new answer in the buffer.
	buffer[pos_buf_cli].ptr_semaphore = new_semaphore; //Puts the new semaphore in the buffer.
	pos_buf_cli = (pos_buf_cli + 1) % buff_size; //Indicates the next position of the buffer to be accessed.

	pthread_mutex_unlock(&mutex_buf_cli[pos_buf_cli]);
	sem_post(&can_consume); //Server can process request.
	sem_wait(&(new_semaphore->sem_answer));	//Blocks itself waiting for the server to process the answer.

	if(new_answer->ret == 0){
		ret_value = (KV_t*)malloc(sizeof(KV_t));  //Allocates memory for the return value.
		ret_value = new_answer->vec; //Copies the address of the vector of pairs to the return value.
		(*dim) = new_answer->n_elem; //Puts the number of elements in the pointes given as argument.
		free(new_request); //Releases the allocated memory for the new request.
		free(new_answer); //Releases the allocated memory for the new answer.
		free(new_semaphore); //Releases the allocated memory for the new semaphore.
		return ret_value;
	}

	free(new_request);
	free(new_answer);
	free(new_semaphore);

	return NULL;
}


