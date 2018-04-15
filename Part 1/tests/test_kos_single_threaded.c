#include <kos_client.h>
#include <stdio.h>
#include <pthread.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define NUM_EL 6
#define NUM_SHARDS 3

int teste=NULL;
KV_t* par;

int main(int argc, const  char* argv[] ) {
	char key[20], value[20];
	char* v;
	int i,j,ret;
	int client_id=1;

	ret=kos_init(1, 1, NUM_SHARDS);


	if (ret!=0)  {
			printf("kos_init failed with code %d!\n",ret);
			return -1;
		}

	for (j=NUM_SHARDS-1; j>=0; j--) {	
		for (i=NUM_EL; i>=0; i--) {
			sprintf(key, "k%d",i);
			sprintf(value, "val:%d",i);
			printf("Element <%s,%s> being inserted in shard %d....", key, value, j);
			fflush(stdin);
			v=kos_put(client_id,j, key,value);
			printf("Element <%s,%s> inserted in shard %d. Prev Value=%s\n", key, value, j, ( v==NULL ? "<missing>" : v ) );
fflush(stdin);
		}
	}

	printf("--------------------------------------------------\n");

	for (j=NUM_SHARDS-1; j>=0; j--) {	
		for (i=0; i<NUM_EL; i++) {
			sprintf(key, "k%d",i);
			v=kos_get(client_id,j, key);
			printf("Element %s %s found in shard %d: value=%s\n", key, ( v==NULL ? "has not been" : "has been" ),j,
									( v==NULL ? "<missing>" : v ) );
	
		}
	}
	
	printf("--------------------------------------------------\n");

	for (j=NUM_SHARDS-1; j>=0; j--) {	
		for (i=NUM_EL; i>=NUM_EL/2; i--) {
			sprintf(key, "k%d",i);
			v=kos_remove(client_id,j, key);
			printf("Element %s %s removed from shard %d. value =%s\n", key, ( v==NULL ? "has not been" : "has been" ),j,
									( v==NULL ? "<missing>" : v ) );
		}
	}

	printf("--------------------------------------------------\n");
	for (j=NUM_SHARDS-1; j>=0; j--) {	
		for (i=0; i<NUM_EL; i++) {
			sprintf(key, "k%d",i);
			v=kos_get(client_id,j, key);
			printf("Element %s %s found in shard %d. value=%s\n", key, ( v==NULL ? "has not been" : "has been" ) ,j,
									( v==NULL ? "<missing>" : v ) );
	
		}
	}

	printf("--------------------------------------------------\n");


	for (j=NUM_SHARDS-1; j>=0; j--) {	
		for (i=0; i<NUM_EL; i++) {
			sprintf(key, "k%d",i);
			sprintf(value, "val:%d",i*10);

			v=kos_put(client_id,j, key,value);
			printf("Element <%s,%s> inserted in shard %d. Prev Value=%s\n", key, value, j, ( v==NULL ? "<missing>" : v ) );
		}
	}

	printf("--------------------------------------------------\n");


	for (j=NUM_SHARDS-1; j>=0; j--) {	
		for (i=0; i<NUM_EL; i++) {
			sprintf(key, "k%d",i);
			v=kos_get(client_id,j, key);
			printf("Element %s %s found in shard %d: value=%s\n", key, ( v==NULL ? "has not been" : "has been" ),j,
									( v==NULL ? "<missing>" : v ) );
	
		}
	}

/*	kos_getAllKeys(1, 1, &teste);
	par=(KV_t*)malloc(teste*sizeof(KV_t));
	par=kos_getAllKeys(1, 1, &teste);
	int z=0;
	for(z=0;z<teste;z++){
		printf("CHAVE:%s.....VALOR:%s", par[z].key, par[z].value);
	}*/

	printf("--------------------------------------------------\n");

	return 0;
}
