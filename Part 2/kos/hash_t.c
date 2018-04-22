#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <hash_t.h>
#include <list.h>

#define HT_SIZE 10

list_t*** hash_t;

int init_hash_t(unsigned int hash) {

	int i, j;
	list_t* new_list;

	hash_t = calloc(hash, sizeof(list_t**));
	for(i = 0; i < hash; i++)
		hash_t[i] = calloc(HT_SIZE, sizeof(list_t*));

	/*Creates a new list for each position of the hash table.*/
	for(i = 0; i < hash; i++)
		for(j = 0; j < HT_SIZE; j++) {
			new_list = lst_new();
			hash_t[i][j] = new_list;
		}
	return 0;
}
