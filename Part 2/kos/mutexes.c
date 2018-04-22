#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define HT_SIZE 10

extern int buff_size;

/*Table of RW-LOCK mutexes that control the accesses of the servers to the hash table.*/
pthread_rwlock_t** mutex_table;

/*Vector of mutexes that control the accesses of the clients to the buffer.*/
pthread_mutex_t* mutex_buf_cli;

/*Vector of mutexes that control the accesses of the servers to the buffer.*/
pthread_mutex_t* mutex_buf_serv;

/*Vector of RWLOCK mutexes that control the access to the files.*/
pthread_rwlock_t* mutex_files;

int init_mutex(int num_shards) {
	int i = 0, j = 0, m = -1;

	mutex_table = (pthread_rwlock_t **)malloc(sizeof(pthread_rwlock_t *) * num_shards);

	/*Allocates memory and intializes the mutexes for each position of each hash table.*/
	for(i = 0; i < num_shards; i++){
		mutex_table[i] = (pthread_rwlock_t *)malloc(sizeof(pthread_rwlock_t) * HT_SIZE);
		for(j = 0; j < HT_SIZE; j++){
			m = pthread_rwlock_init(&mutex_table[i][j], NULL);

			if(m != 0) {
				printf("Error creating the mutexes of access to the hash table.\n");
				return -1;
			}
		}
	}

	/*Allocates memory and initializes the mutexes that allow clients to have access to the buffer, according with the size of the buffer.*/
	mutex_buf_cli = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * buff_size);

	for(i = 0; i < buff_size; i++){
		m = pthread_mutex_init(&mutex_buf_cli[i], NULL);

		if(m !=0) {
			printf("Error creating the mutexes that allow clients to have access to the buffer.\n");
			return -1;
		}
	}

	/*Allocates memory and intializes the mutexes that allow servers to have access to the buffer, according with the size of the buffer.*/
	mutex_buf_serv = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * buff_size);

	for(i = 0; i < buff_size; i++){
		m = pthread_mutex_init(&mutex_buf_serv[i], NULL);

		if(m != 0) {
			printf("Error creating the mutexes that allow servers to have access to the buffer.\n");
			return -1;
		}
	}


	/*Allocates memory and initializes the mutexes that access the files, according with the number of shards.*/
	mutex_files = (pthread_rwlock_t*)malloc(sizeof(pthread_rwlock_t) * num_shards);

	for(i = 0; i < num_shards; i++){
		m = pthread_rwlock_init(&mutex_files[i], NULL);

		if(m != 0){
			printf("Error creating the mutexes that access the files.\n");
			return -1;
		}
	}


	return 0;
}
