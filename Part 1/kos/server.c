#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#include <buffer.h>
#include <server.h>
#include <list.h>
#include <hash_t.h>
#include <hash.c>

extern info* buffer; //Buffer.
extern list_t*** hash_t; //Hash Table with lists of pairs KV_T.
extern int buff_size;

/*Vector of threads for the servers.*/
pthread_t* servers;

/*Semaphore that controls the accesses to the hash table.*/
sem_t sem_hash;

int init_servers(int num_server_threads) {
	int i;
	int* ids = (int*) malloc(sizeof(int)*num_server_threads); //Allocates memory for the ids of the servant threads.

	sem_init(&sem_hash, 0, 1); //Initializes the semaphore that controls the accesses to the hash table.

	servers = (pthread_t*) malloc(sizeof(pthread_t)*num_server_threads); //Allocates memory for the ids of the servant threads.

	for(i = 0; i < num_server_threads; i++){
		ids[i] = i;
		servers[i] = pthread_create(&servers[i], NULL, server_t, &(ids[i])); //Creates the servant tasks.

		if(servers[i] != 0){
			printf("Error creating the servers!\n");
			return -1;
		}
	}
	return 0;
}

void* server_t(void* ptr) {
	int thread_id =* ((int*)ptr);
	int p_buff; //Position of the buffer to access.
	int p_shard; //Position in the shard corresponding to the key.
	request* req; //Request in the buffer.
	answer* ans; //Answer in the buffer.
	KV_t* pair = NULL; //Pointer for a 'pair'.
	list_t* list = NULL; //Pointer for the intended list of the hash table.
	int n_elem = 0, i = 0, j = 0;
	KV_t* pair_vector = NULL;

	p_buff = thread_id % buff_size;	//Position of the buffer to access.

	while(1) {
		sem_wait(&(buffer[p_buff].ptr_semaphores->sem_server)); //Server blocks waiting for the client.
		req = buffer[p_buff].ptr_request; //Copies the address of the request to the buffer.
		ans = buffer[p_buff].ptr_answer; //Copies the address of the answer to the buffer.

		/*Deals with the information depending on the client's command.*/
		switch(req->command) {
			/*kos_get*/
			case 1:
				p_shard = hash(req->key); //Checks which is the position of the hash table to be accessed.
				list = hash_t[req->shardId][p_shard]; //Points to the intended list.
				sem_wait(&sem_hash); //Prevents the access to the hash table.
				pair = lst_search(list, req->key); //Searches for the pair with the corresponding key.
				sem_post(&sem_hash); //Allows the access to the hash table.

				if(pair != NULL){
					strcpy(ans->value, (*pair).value); //If exists, copies the 'value' corresponding to the 'key'.
				} else {
					ans->ret = -1; //If does not exist, puts '-1' in the return value.
				}
				sem_post(&(buffer[p_buff].ptr_semaphores->sem_client)); //"Tells" the client that it can make a new request.
				break;

			/*kos_put*/
			case 2:
				p_shard=hash(req->key); //Checks which is the position of the hash table to be accessed.
				list=hash_t[req->shardId][p_shard];	//Points to the intended list.
				sem_wait(&sem_hash); //Prevents the access to the hash table.
				pair=lst_search(list, req->key); //Searches for the pair with the corresponding key.
				sem_post(&sem_hash); //Allows the access to the hash table.

				if(pair!=NULL) {
					strcpy(ans->value,(*pair).value); //Copies the 'value' corresponding to the 'key'.
					sem_wait(&sem_hash); //Prevents the access to the hash table.
					lst_remove(list, req->key); //Removes the element that already exists on the list.
					sem_post(&sem_hash); //Allows the access to the hash table.
				} else {
					ans->ret = -1; //If does not exist, puts '-1' in the return value.
				}
				sem_wait(&sem_hash); //Prevents the access to the hash table.
				lst_insert(list, req->key, req->value); //Puts the new element on the list.
				sem_post(&sem_hash); //Allows the access to the hash table.
				sem_post(&(buffer[p_buff].ptr_semaphores->sem_client));	//"Tells" the client that it can make a new request.
				break;

			/*kos_remove*/
			case 3:
				p_shard = hash(req->key); //Checks which is the position of the hash table to be accessed.
				list=hash_t[req->shardId][p_shard];	//Points to the intended list.
				sem_wait(&sem_hash); //Prevents the access to the hash table.
				pair = lst_search(list, req->key); //Searches for the pair with the corresponding key.
				sem_post(&sem_hash); //Allows the access to the hash table.

				if(pair != NULL) {
					strcpy(ans->value, (*pair).value); //Copies the 'value' corresponding to the 'key'.
					sem_wait(&sem_hash); //Prevents the access to the hash table.
					lst_remove(list, req->key); //Removes the element that already exists on the list.
					sem_post(&sem_hash); //Allows the access to the hash table.
				} else {
					ans->ret = -1; //If does not exist, puts '-1' in the return value.
				}
				sem_post(&(buffer[p_buff].ptr_semaphores->sem_client)); //"Tells" the client that it can make a new request.
				break;

			/*kos_getAllKeys*/
			case 4:
				n_elem = 0;	//Initializes the variable used to save the number of elements of the list.
				j = 0; //Initializes the variable used to go through the vector of pairs.	
	
				for(p_shard = 0; p_shard < HT_SIZE; p_shard++) { //Counts the number of elements of the shard.
					sem_wait(&sem_hash); //Prevents the access to the hash table.
					n_elem += lst_count(hash_t[req->shardId][p_shard]);
					sem_post(&sem_hash); //Allows the access to the hash table.
				}

				//If the shard has no elements.
				if(n_elem == 0){
					ans->ret = -1;
					break;
				}

				//Allocates memory for the vector of pairs.
				pair_vector = (KV_t*) malloc(n_elem*sizeof(KV_t));		

				//Goes through all the positions of the shard.
				for(p_shard = 0; p_shard < HT_SIZE; p_shard++) {
					sem_wait(&sem_hash); //Prevents the access to the hash table.
					i=0;
					list = hash_t[req->shardId][p_shard];

					//Goes through the list 'list' of the position 'p_shard' and saves it's elements.
					while(lst_getPair(list, i) != NULL) {
						strcpy(pair_vector[j].key, (lst_getPair(list, i))->key);
						strcpy(pair_vector[j].value, (lst_getPair(list, i))->value);
						i++;
						j++;
					}
					sem_post(&sem_hash); //Prevents the access to the hash table.
				}

				ans->vec = pair_vector;	//Puts the vector of pairs in the answer for the client.
				ans->n_elem = n_elem; //Puts the number of elements in the answer for the client.
				sem_post(&(buffer[p_buff].ptr_semaphores->sem_client)); //"Tells" the client that it can make a new request.
				break;
		}
	}
}
