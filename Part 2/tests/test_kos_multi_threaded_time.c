

#include <kos_client.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NUM_EL 5
#define NUM_SHARDS 3
#define NUM_CLIENT_THREADS 3
#define NUM_SERVER_THREADS 3

#define KEY_SIZE 20

// #define DEBUG_PRINT_ENABLED 1  // uncomment to enable DEBUG statements
#if DEBUG_PRINT_ENABLED
#define DEBUG printf
#else
#define DEBUG(format, args...) ((void)0)
#endif


/* Suplements the key to bypass the hash function.
k client_id, l iteration */
int de_hash(int k, int l) {
        int y;

        y = (l+10)*10000 + ((99-(k+10))*100+(99-(l+10)));
        return y;
}




void *client_thread(void *arg) {

	char key[KEY_SIZE], value[KEY_SIZE];
	char* v;
	int i;
	long now, time();
	char *ctime();
	int client_id=*( (int*)arg);
	KV_t* dump;


/* 3 different sequences put-get-remove:
	1 - each client thread invokes a different shard
	2 - all client threads invoke the same shard
	3 - all client threads colide in a few list of the same shard */


/* 1st sequence - Each client thread generates requests for its "own" different shard */
	if(client_id==0) {
		time(&now);
		printf("STEP 1 - EACH CLIENT REQUESTS ITS OWN SHARD @ %s\n", ctime (&now));
	}


	for (i=NUM_EL-1; i>=0; i--) {
		sprintf(key, "k-c%d-%d",client_id,i);
		sprintf(value, "val:%d",i);
		v=kos_put(client_id, client_id%NUM_SHARDS, key,value);
		DEBUG("C:%d  <%s,%s> inserted in shard %d. Prev Value=%s\n", client_id, key, value, j, ( v==NULL ? "<missing>" : v ) );
	}

	printf("------------------- %d:1/7 ENDED INSERTING -----------------------\n",client_id);

	for (i=0; i<NUM_EL; i++) {
		sprintf(key, "k-c%d-%d",client_id,i);
		sprintf(value, "val:%d",i);
		v=kos_get(client_id, client_id%NUM_SHARDS, key);
		if (strncmp(v,value,KEY_SIZE)!=0) {
			printf("Error on key %s value should be %s and was returned %s",key,value,v);
			exit(1);
		}
		DEBUG("C:%d  %s %s found in shard %d: value=%s\n", client_id, key, ( v==NULL ? "has not been" : "has been" ),client_id,
								( v==NULL ? "<missing>" : v ) );	
	}

	
	printf("------------------ %d:2/7 ENDED READING  ---------------------\n",client_id);


	for (i=NUM_EL-1; i>=0; i--) {
		sprintf(key, "k-c%d-%d",client_id,i);
		sprintf(value, "val:%d",i);
		v=kos_remove(client_id, client_id%NUM_SHARDS, key);
		if (strncmp(v,value,KEY_SIZE)!=0) {
			printf("Error when removing key %s value should be %s and was returned %s",key,value,v);
			exit(1);
		}
		DEBUG("C:%d  %s %s removed from shard %d. value =%s\n", client_id, key, ( v==NULL ? "has not been" : "has been" ),client_id,
								( v==NULL ? "<missing>" : v ) );
	}



	printf("----------------- %d-4/7 ENDED REMOVING -------------------------\n",client_id);



/* 2nd sequence - All client threads generate requests for the same shard (0) */
if(client_id==0) {
		time(&now);
		printf("STEP 2 - ALL CLIENTS REQUEST THE SAME SHARD @ %s\n", ctime (&now));
	}



	for (i=NUM_EL-1; i>=0; i--) {
		sprintf(key, "k-c%d-%d",client_id,i);
		sprintf(value, "val:%d",i);
		v=kos_put(client_id, 0, key,value);
		DEBUG("C:%d  <%s,%s> inserted in shard %d. Prev Value=%s\n", client_id, key, value, j, ( v==NULL ? "<missing>" : v ) );
	}

	printf("------------------- %d:1/7 ENDED INSERTING -----------------------\n",client_id);

	for (i=0; i<NUM_EL; i++) {
		sprintf(key, "k-c%d-%d",client_id,i);
		sprintf(value, "val:%d",i);
		v=kos_get(client_id, 0, key);
		if (strncmp(v,value,KEY_SIZE)!=0) {
			printf("Error on key %s value should be %s and was returned %s",key,value,v);
			exit(1);
		}
		DEBUG("C:%d  %s %s found in shard %d: value=%s\n", client_id, key, ( v==NULL ? "has not been" : "has been" ),client_id,
								( v==NULL ? "<missing>" : v ) );	
	}

	
	printf("------------------ %d:2/7 ENDED READING  ---------------------\n",client_id);


	for (i=NUM_EL-1; i>=0; i--) {
		sprintf(key, "k-c%d-%d",client_id,i);
		sprintf(value, "val:%d",i);
		v=kos_remove(client_id, 0, key);
		if (strncmp(v,value,KEY_SIZE)!=0) {
			printf("Error when removing key %s value should be %s and was returned %s",key,value,v);
			exit(1);
		}
		DEBUG("C:%d  %s %s removed from shard %d. value =%s\n", client_id, key, ( v==NULL ? "has not been" : "has been" ),client_id,
								( v==NULL ? "<missing>" : v ) );
	}



	printf("----------------- %d-4/7 ENDED REMOVING -------------------------\n",client_id);




/* 3rd sequence - All client threads generate requests for the same list in the same shard - more contention */
if(client_id==0) {
		time(&now);
		printf("STEP 3 - ALL CLIENTS REQUEST THE SAME LIST IN THE SAME SHARD @ %s\n", ctime (&now));
	}



	for (i=NUM_EL-1; i>=0; i--) {
		sprintf(key, "k-c%d-%d",client_id+10, de_hash(client_id,i));
		sprintf(value, "val:%d",i);
		v=kos_put(client_id, 0, key,value);
		DEBUG("C:%d  <%s,%s> inserted in shard %d. Prev Value=%s\n", client_id, key, value, j, ( v==NULL ? "<missing>" : v ) );
	}

	printf("------------------- %d:1/7 ENDED INSERTING -----------------------\n",client_id);

	for (i=0; i<NUM_EL; i++) {
		sprintf(key, "k-c%d-%d",client_id+10, de_hash(client_id,i));
		sprintf(value, "val:%d",i);
		v=kos_get(client_id, 0, key);
		if (strncmp(v,value,KEY_SIZE)!=0) {
			printf("Error on key %s value should be %s and was returned %s",key,value,v);
			exit(1);
		}
		DEBUG("C:%d  %s %s found in shard %d: value=%s\n", client_id, key, ( v==NULL ? "has not been" : "has been" ),client_id,
								( v==NULL ? "<missing>" : v ) );	
	}

	
	printf("------------------ %d:2/7 ENDED READING  ---------------------\n",client_id);


	for (i=NUM_EL-1; i>=0; i--) {
		sprintf(key, "k-c%d-%d",client_id+10, de_hash(client_id,i));
		sprintf(value, "val:%d",i);
		v=kos_remove(client_id, 0, key);
		if (strncmp(v,value,KEY_SIZE)!=0) {
			printf("Error when removing key %s value should be %s and was returned %s",key,value,v);
			exit(1);
		}
		DEBUG("C:%d  %s %s removed from shard %d. value =%s\n", client_id, key, ( v==NULL ? "has not been" : "has been" ),client_id,
								( v==NULL ? "<missing>" : v ) );
	}



	printf("----------------- %d-4/7 ENDED REMOVING -------------------------\n",client_id);




	return NULL;
}


int main(int argc, const  char* argv[] ) {

	int i,s,ret;
	int* res;
 	long now, time();
     	char *ctime();
	pthread_t* threads=(pthread_t*)malloc(sizeof(pthread_t)*NUM_CLIENT_THREADS);
	int* ids=(int*) malloc(sizeof(int)*NUM_CLIENT_THREADS);

	// this test uses NUM_CLIENT_THREADS shards, as, in some tests, each client executes commands in its own shard
	ret=kos_init(NUM_CLIENT_THREADS,NUM_SERVER_THREADS,NUM_CLIENT_THREADS);

	//printf("KoS inited");

	if (ret!=0)  {
			printf("kos_init failed with code %d!\n",ret);
			return -1;
		}
		
	for (i=0; i<NUM_CLIENT_THREADS; i++) {	
		ids[i]=i;		
		
		if ( (s=pthread_create(&threads[i], NULL, &client_thread, &(ids[i])) ) ) {
			printf("pthread_create failed with code %d!\n",s);
			return -1;
		}
	}
	time(&now);
	printf("BEGIN @ %s\n", ctime(&now));
	for (i=0; i<NUM_CLIENT_THREADS; i++) {	
               s = pthread_join(threads[i], (void**) &res);
               if (s != 0) {
                   printf("pthread_join failed with code %d",s);
			return -1;
		}
           }
	time(&now);
	printf("END   @ %s\n", ctime(&now));
	printf("\n--> TEST PASSED <--\n");

	return 0;
}

