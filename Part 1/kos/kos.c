#include <kos_client.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

#include <server.h>
#include <buffer.h>
#include <hash_t.h>
#include <list.h>

#define KV_SIZE 20

extern list_t*** hash_t; //Hash Table with lists of pairs KV_T.
extern info *buffer; //Buffer.
extern int buff_size; //Size of the buffer.

int counter; //Numver of keys in the shard requested by kos_getAllKeys;

int kos_init(int num_server_threads, int buf_size, int num_shards) {

	int success = 0; //Variable that saves the "success" of each function.

	init_hash_t(num_shards); //Initializes the hash table.

	success = init_buffer(buf_size); //Initializes the buffer.

	if(success != 0)
		return -1;

	success = init_servers(num_server_threads);	//Initializes the servers.

	if(success != 0)
		return -1;
	return 0;
}

char* kos_get(int clientid, int shardId, char* key) {
	request* new_request;
	answer* new_answer;
	char* ret_value = NULL;
	int p_buff; //Position of the buffer to access.

	new_request = (request*) malloc(sizeof(request)); //Allocates memory for the new request.
	strcpy(new_request->key, key); //Puts the key in the request.
	memset(new_request->value, '\0', sizeof(new_request->value));	//"Cleans" the string 'value' of the new request.
	new_request->shardId = shardId; //Puts the id of the shard in the request.
	new_request->command = 1; //Command 1: 'kos_get'.

	new_answer = (answer*) malloc(sizeof(answer)); //Allocates memory for the new answer.
	memset(new_answer->value, '\0', sizeof(new_answer->value)); //"Cleans" the string 'value' of the new answer.
	new_answer->ret = 0; //Puts 0 in the return value.

	p_buff = clientid % buff_size; //Checks what is the position of the buffer to be accessed.
	buffer[p_buff].ptr_request = new_request; //Puts the new request in the buffer.
	buffer[p_buff].ptr_answer = new_answer;	//Puts the new answer in the buffer.

	sem_post(&(buffer[p_buff].ptr_semaphores->sem_server)); //Tells the server that it can analyze the request.
	sem_wait(&(buffer[p_buff].ptr_semaphores->sem_client));	//Waits for the server to finish analyze the request.

	if(new_answer->ret == 0) {
		ret_value = (char*) malloc(KV_SIZE*sizeof(char)); //Allocates memory for the return value.
		strcpy(ret_value, new_answer->value); //Copies the value of 'value' to the return variable.
		free(new_request); //Releases memory allocated for the new request.
		free(new_answer); //Releases memory allocated for the new answer.
		return ret_value;
	}

	return NULL;
}

char* kos_put(int clientid, int shardId, char* key, char* value) {
	request* new_request;
	answer* new_answer;
	char* ret_value = NULL;
	int p_buff;	//Position of the buffer to access.

	new_request = (request*) malloc(sizeof(request));	//Allocates memory for the new request.
	strcpy(new_request->key, key); //Puts the key in the request.
	strcpy(new_request->value, value); //Puts the value in the request.
	new_request->shardId = shardId; //Puts the id of the shard in the request.
	new_request->command = 2; //Command 2: 'kos_put'.

	new_answer = (answer*) malloc(sizeof(answer)); //Allocates memory for the new answer.
	memset(new_answer->value, '\0', sizeof(new_answer->value)); //"Cleans" the string 'value' of the new answer.
	new_answer->ret = 0; //Puts 0 in the return value.

	p_buff = clientid % buff_size; //Checks what is the position of the buffer to be accessed.
	buffer[p_buff].ptr_request = new_request; //Puts the new request in the buffer.
	buffer[p_buff].ptr_answer = new_answer;	//Puts the new answer in the buffer.

	sem_post(&(buffer[p_buff].ptr_semaphores->sem_server));	//Tells the server that it can analyze the request.
	sem_wait(&(buffer[p_buff].ptr_semaphores->sem_client));	//Waits for the server to finish analyze the request.

	if(new_answer->ret==0) {
		ret_value=(char*) malloc(KV_SIZE*sizeof(char)); //Allocates memory for the return value.
		strcpy(ret_value, new_answer->value); //Copies the value of 'value' to the return variable
		free(new_request); //Releases memory allocated for the new request.
		free(new_answer); //Releases memory allocated for the new answer.
		return ret_value;
	}

	return NULL;
}

char* kos_remove(int clientid, int shardId, char* key) {
	request* new_request;
	answer* new_answer;
	char* ret_value = NULL;;
	int p_buff; //Position of the buffer to access.

	new_request = (request*) malloc(sizeof(request)); //Allocates memory for the new request.
	strcpy(new_request->key, key); //Puts the key in the request.
	memset(new_request->value, '\0', sizeof(new_request->value)); //"Cleans" the string 'value' of the new request.
	new_request->shardId = shardId;	//Puts the id of the shard in the request.
	new_request->command = 3; //Command 3: 'kos_remove'.

	new_answer=(answer*) malloc(sizeof(answer)); //Allocates memory for the new answer.
	memset(new_answer->value, '\0', sizeof(new_answer->value)); //"Cleans" the string 'value' of the new answer.
	new_answer->ret = 0; //Puts 0 in the return value.

	p_buff = clientid % buff_size; //Checks what is the position of the buffer to be accessed.
	buffer[p_buff].ptr_request = new_request; //Puts the new request in the buffer.
	buffer[p_buff].ptr_answer = new_answer; //Puts the new answer in the buffer.

	sem_post(&(buffer[p_buff].ptr_semaphores->sem_server)); //Tells the server that it can analyze the request.
	sem_wait(&(buffer[p_buff].ptr_semaphores->sem_client));	//Waits for the server to finish analyze the request.

	if(new_answer->ret == 0) {
		ret_value = (char*) malloc(KV_SIZE*sizeof(char)); //Allocates memory for the return value.
		strcpy(ret_value, new_answer->value); //Copies the value of 'value' to the return variable
		free(new_request); //Releases memory allocated for the new request.
		free(new_answer); //Releases memory allocated for the new answer.
		return ret_value;
	}

	return NULL;
}

KV_t* kos_getAllKeys(int clientid, int shardId, int* dim) {
	request* new_request;
	answer* new_answer;
	KV_t* ret_value = NULL;;
	int p_buff; //Position of the buffer to access.

	new_request = (request*) malloc(sizeof(request)); //Allocates memory for the new request.
	memset(new_request->key, '\0', sizeof(new_request->key)); //"Cleans" the string 'key' of the new request.
	memset(new_request->value, '\0', sizeof(new_request->value)); //"Cleans" the string 'value' of the new request.
	new_request->shardId = shardId;	//Puts the id of the shard in the request.
	new_request->command = 4; //Command 4: 'kos_getAllKeys'.

	new_answer = (answer*) malloc(sizeof(answer)); //Allocates memory for the new answer.
	memset(new_answer->value, '\0', sizeof(new_answer->value)); //"Cleans" the string 'value' of the new answer.
	new_answer->vec = NULL;
	new_answer->ret = 0; //Puts 0 in the return value.

	p_buff = clientid % buff_size; //Checks what is the position of the buffer to be accessed.
	buffer[p_buff].ptr_request = new_request; //Puts the new request in the buffer.
	buffer[p_buff].ptr_answer = new_answer; //Puts the new answer in the buffer.

	sem_post(&(buffer[p_buff].ptr_semaphores->sem_server));	//Tells the server that it can analyze the request.
	sem_wait(&(buffer[p_buff].ptr_semaphores->sem_client));	//Waits for the server to finish analyze the request.

	if(new_answer->ret == 0) {
		ret_value = (KV_t*) malloc(sizeof(KV_t)); //Allocates memory for the return value.
		ret_value = new_answer->vec; //Copies the address of the vector of pairs to the return value.
		(*dim) = new_answer->n_elem; //Puts the number of elements in the pointer passed as argument.
		free(new_request); //Releases memory allocated for the new request.
		free(new_answer); //Releases memory allocated for the new answer.
		return ret_value;
	}

	return NULL;
}


