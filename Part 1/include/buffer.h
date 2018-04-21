#include <semaphore.h>
#include <kos_client.h>

#define KV_SIZE 20

typedef struct request {
	char key[KV_SIZE];
	char value[KV_SIZE];
	int shardId;
	int command; //1,2,3,4 accordingly to the command.
} request;

typedef struct answer {
	char value[KV_SIZE];
	int ret;
	int n_elem;
	KV_t* vec;
} answer;

typedef struct semaphores {
	sem_t sem_client;
	sem_t sem_server;
} semaphores;

typedef struct info {
	request* ptr_request;
	answer* ptr_answer;
	semaphores* ptr_semaphores;	
} info;

/*Initializes the buffer*/
int init_buffer(int buff);
