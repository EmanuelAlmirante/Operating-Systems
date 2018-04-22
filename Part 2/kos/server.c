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
#include <hash.h>
#include <delay.h>
#include <mutexes.h>
#include <files.h>

#define HT_SIZE 10

extern info* buffer;
extern list_t*** hash_t;
extern int buff_size;
extern sem_t can_produce;
extern sem_t can_consume;
extern pthread_mutex_t* mutex_buf_serv;
extern pthread_rwlock_t** mutex_table;
extern pthread_rwlock_t* mutex_files;

/*Vector of threads for the servers.*/
pthread_t* servers;

/*Position of the buffer that which server will access.*/
int pos_buf_ser = 0;

/*Number of blank spaces in a file.*/
int* ins_rem;


int init_servers(int num_shards, int num_server_threads) {
	int i = 0, s = -1;
	int* ids = (int*)malloc(sizeof(int)*num_server_threads); //Allocates memory for the ids of the serving threads.

	ins_rem = (int*)malloc(sizeof(int)*num_shards);
	memset(ins_rem, 0, sizeof(int)*num_shards);

	servers = (pthread_t*) malloc(sizeof(pthread_t)*num_server_threads); //Allocates memory for the serving threads.

	for(i = 0; i < num_server_threads; i++) {
		ids[i] = i;
		s = pthread_create(&servers[i], NULL, server_t, &(ids[i]));	//Creates the serving tasks.

		if(s != 0) {
			printf("Error creating the servers.\n");
			return -1;
		}
	}

	return 0;
}

void* server_t(void* ptr) {
	int p_shard; //Position of the shard that corresponds to the key.
	request* req; //Request in the buffer.
	answer* ans; //Answer in the buffer.
	semaphore* sem_res; //Semaphore in the buffer.
	KV_t* pair = NULL; //Pointer to a 'pair'.
	list_t* list = NULL; //Pointes to the intended list of the hash table.
	int n_elem = 0, i = 0, j = 0;
	KV_t* pair_vector = NULL;

	while(1) {
		sem_wait(&can_consume); //Blocks itself waiting for a request.
		pthread_mutex_lock(&mutex_buf_serv[pos_buf_ser]);
		delay();

		req = buffer[pos_buf_ser].ptr_request; //Copies the address of the request of the buffer.
		ans = buffer[pos_buf_ser].ptr_answer; //Copies the address of the answer of the buffer.
		sem_res = buffer[pos_buf_ser].ptr_semaphore; //Copies the address of the semaphore of the buffer.
		pos_buf_ser = (pos_buf_ser + 1) % buff_size; //Next position of the buffer to access.

		pthread_mutex_unlock(&mutex_buf_serv[pos_buf_ser]);
		sem_post(&can_produce); //Can put a new request in the current position of the buffer.

		/*Deals with the information depending on the command of the client.*/
		switch(req->command) {
			/*kos_get*/
			case 1:
				delay();
				p_shard = hash(req->key); //Checks which is the position of the hash table to access.
				list = hash_t[req->shardId][p_shard]; //Points to the intendend list.
				pthread_rwlock_rdlock(&mutex_table[req->shardId][p_shard]);
				delay();
				pair = lst_search(list, req->key); //Searches for the pair with the corresponding key.
				pthread_rwlock_unlock(&mutex_table[req->shardId][p_shard]);

				if(pair != NULL) {
					strcpy(ans->value, (*pair).value); //If it exists, copies the 'value' corresponding to the 'key'.
				} else {
					ans->ret = -1; //If does not exist, puts '-1' in the return value.
				}
				sem_post(&sem_res->sem_answer); //"Tells" the client that already processed the request.
				break;

			/*kos_put*/
			case 2:
				delay();
				p_shard = hash(req->key); //Checks which is the position of the hash table to access.
				list=hash_t[req->shardId][p_shard];	//Points to the intended list.
				pthread_rwlock_rdlock(&mutex_table[req->shardId][p_shard]);
				delay();
				pair = lst_search(list, req->key); //Searches for the pair with the corresponding key.
				pthread_rwlock_unlock(&mutex_table[req->shardId][p_shard]);

				if(pair != NULL){
					strcpy(ans->value, (*pair).value); //Copies the 'value' corresponding to the 'key'.
					pthread_rwlock_wrlock(&mutex_table[req->shardId][p_shard]);
					delay();
					lst_remove(list, req->key); //Removes the existing element from the list.
					pthread_rwlock_unlock(&mutex_table[req->shardId][p_shard]);

					pthread_rwlock_wrlock(&mutex_files[req->shardId]);
					delay();
					removes_elem(req->shardId, req->key); //Removes the existing element from the file.
					ins_rem[req->shardId]--;
					pthread_rwlock_unlock(&mutex_files[req->shardId]);
				} else {
					ans->ret = -1; //If it does not exist, puts '-1' in the return value.
				}
				pthread_rwlock_wrlock(&mutex_table[req->shardId][p_shard]);
				delay();
				lst_insert(list, req->key, req->value); //Inserts the new element in the list.
				pthread_rwlock_unlock(&mutex_table[req->shardId][p_shard]);

				pthread_rwlock_wrlock(&mutex_files[req->shardId]);
				delay();
				writes_elem(req->shardId, req->key, req->value); //Inserts the new element in the file.
				ins_rem[req->shardId]++;
				pthread_rwlock_unlock(&mutex_files[req->shardId]);
				sem_post(&sem_res->sem_answer); //"Tells" the client that already processed the request.
				break;

			/*kos_remove*/
			case 3:
				delay();
				p_shard = hash(req->key); //Checks which is the position of the hash table to access.
				list = hash_t[req->shardId][p_shard]; //Points to the intended list.
				pthread_rwlock_rdlock(&mutex_table[req->shardId][p_shard]);
				delay();
				pair = lst_search(list, req->key); //Searches for the pair with the corresponding key.
				pthread_rwlock_unlock(&mutex_table[req->shardId][p_shard]);

				if(pair != NULL){
					strcpy(ans->value, (*pair).value); //Copies the 'value' corresponding to the 'key'.
					pthread_rwlock_wrlock(&mutex_table[req->shardId][p_shard]);
					delay();
					lst_remove(list, req->key);	//Removes the existing element from the list.
					pthread_rwlock_unlock(&mutex_table[req->shardId][p_shard]);

					pthread_rwlock_wrlock(&mutex_files[req->shardId]);
					delay();
					removes_elem(req->shardId, req->key); //Removes the existing element from the file.
					ins_rem[req->shardId]--;
					//If the number of removes is greater than the number of insertes, compacts the file.
					if(ins_rem[req->shardId] < 0) {
						compacts_file(req->shardId);
					}
					pthread_rwlock_unlock(&mutex_files[req->shardId]);
				} else {
					ans->ret = -1; //If it does not exist, it puts '-1' in the return value.
				}
				sem_post(&sem_res->sem_answer); //"Tells" the client that already processed the request.
				break;

			/*kos_getAllKeys*/
			case 4:
				delay();
				n_elem = 0; //Initializes the variable used to save the number of elements of the list.
				j = 0; //Initializes the variable used to go through the vector of pairs.	
	
				for(p_shard = 0; p_shard < HT_SIZE; p_shard++){ //Counts the number of elements in the shard.
					pthread_rwlock_rdlock(&mutex_table[req->shardId][p_shard]);
					delay();
					n_elem += lst_count(hash_t[req->shardId][p_shard]);
					pthread_rwlock_unlock(&mutex_table[req->shardId][p_shard]);
				}

				//In case the shard has no elements.
				if(n_elem == 0) {
					ans->ret = -1;
					break;
				}

				pair_vector=(KV_t*)malloc(n_elem*sizeof(KV_t)); //Allocates memory for the vector of pairs.

				//Goes through all positions of the shard Percorre todas as posicoes da shard.
				for(p_shard = 0; p_shard < HT_SIZE; p_shard++){
					i = 0;
					list = hash_t[req->shardId][p_shard];

					//Goes through the list 'list' of the position 'p_shard' of the shard and saves it's elements.
					pthread_rwlock_rdlock(&mutex_table[req->shardId][p_shard]);
					while(lst_getPair(list, i) != NULL){
						delay();
						strcpy(pair_vector[j].key, (lst_getPair(list, i))->key);
						strcpy(pair_vector[j].value, (lst_getPair(list, i))->value);
						i++; //Next element.
						j++;
					}
					pthread_rwlock_unlock(&mutex_table[req->shardId][p_shard]);
				}

				ans->vec = pair_vector;//Puts the vector of pairs in the answer for the client.
				ans->n_elem = n_elem; //Puts the number of elements in the answer for the client.
				sem_post(&sem_res->sem_answer); //"Tells" the client that already processed the request.
				break;
		}
	}
}
