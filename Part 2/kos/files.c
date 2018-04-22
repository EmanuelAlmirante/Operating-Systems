#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <list.h>
#include <hash_t.h>
#include <hash.h>

#define HT_SIZE 10 //Max size of the hash table.
#define KV_SIZE 20 //Max size of each key/value.
#define N_CHAR_MAX (KV_SIZE*2+4) //Max number of digits that each line can have.
#define SPACE " "
#define END_LINE "\n"

extern list_t*** hash_t;

int* filed; //Vector that saves the pointers to the files descriptors.

int init_files(int num_files) {

	int i = 0;
	char fname[16];

	filed = (int*) malloc(sizeof(int)*num_files); //Allocates memory for the files descriptors.

	for(i = 0; i < num_files; i++) {
		sprintf(fname, "fShard%d", i); //Name of the file.
		filed[i] = open(fname, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); //Opens the file to read and write. If it does not exist, it is created.
		if(filed[i] < 0) {
			printf("Error opening the file: %s \n", fname);
			return -1;
		}
	}

	return 0;
}

int imports_file(int num_files) {
	
	int i = 0;
	char line[N_CHAR_MAX];
	char c[2];
	char* key = NULL;
	char* value = NULL;

	for(i = 0; i < num_files; i++) {
   		memset(line, 0, sizeof(line)); //Puts the variable equal to 0's.
   		memset(c, 0, sizeof(c)); //Puts the variable equal to 0's.

		lseek(filed[i], 0L, SEEK_SET); //Points to que beginning of the file.

		while (read(filed[i], c, 1) > 0){ //Ignores the first line.
			if(strcmp(c, END_LINE) == 0 || c == NULL) {
				break;
			}
		}
		
		while (read(filed[i], c, 1) > 0) { //Reads file.
			if(strcmp(c,END_LINE) != 0) {		
				strcat(line,c); //Saves in 'line' each line of the file, except the first one.
			}else {
				key = NULL;
				value = NULL;
				key = strtok(line, "<,>\n"); //Reads the value of the "key" of the line.
				value = strtok(NULL, "<,>\n"); //Reads the value of the "key" of the line.
				if(key == NULL || value == NULL) { //If did not read correctly, ignores.
					continue;
				}
				lst_insert(hash_t[i][hash(key)], key, value); //Puts in the respective list.
  				memset(line, 0, sizeof(line));
			}
		}
	}

	return 0;
}

int writes_elem(int shardId, char* key, char* value) {

	char line[N_CHAR_MAX];
	char num_pairs[N_CHAR_MAX];
	char c[2];
	int n = 0;
	int i = 0, j = 0;
	int tam = 0;

	memset(line, 0, sizeof(line));
   	memset(num_pairs, 0, sizeof(num_pairs));
   	memset(c, 0, sizeof(c));

	lseek(filed[shardId], 0L, SEEK_SET); //Points to que beginning of the file.

	while (read(filed[shardId], c, 1) > 0) { //Reads the number of pairs already present in the file, if there is any.
		if(strcmp(c,SPACE) == 0){
			break;
		}
		strcat(num_pairs, c);
	}

	n = atoi(num_pairs);
	n++; //Increases the number of pairs.

	//Updates the variable with the new number of pairs.
	sprintf(num_pairs, "%d", n);	
	tam = strlen(num_pairs);
	for(i = tam; i < N_CHAR_MAX-2; i++){
		strcat(num_pairs, SPACE);
	}
	strcat(num_pairs, END_LINE);

	lseek(filed[shardId], 0L, SEEK_SET); //Points to que beginning of the file.
	write(filed[shardId], num_pairs, strlen(num_pairs)); //Writes the new number of pairs.

	//Puts the pair in the variable "line".
	strcat(line, "<");
	strcat(line, key);
	strcat(line, ",");
	strcat(line, value);
	strcat(line, ">");
	for(i = strlen(line); i < N_CHAR_MAX-2; i++){
		strcat(line, SPACE);
	}
	strcat(line, END_LINE);

	/*If there are deleted lines, puts in the first that finds.*/
	while(read(filed[shardId], c, 1) > 0) {
		if(strcmp(c, SPACE) == 0) {
			j++;
		} else if(strcmp(c, END_LINE) == 0 && j == N_CHAR_MAX-2) {
			lseek(filed[shardId], -N_CHAR_MAX+1, SEEK_CUR); //Points to where it wants to write.
			write(filed[shardId], line, strlen(line)); //Writes in the file the existing pair in "line".
			return 0;
		} else {
			j = 0;
		}
	}

	lseek(filed[shardId], 0, SEEK_END); //Points to the end of the file.
	write(filed[shardId], line, strlen(line)); //Writes in the file the existing pair in "line".

	return 0;
}

int removes_elem(int shardId, char* key) {
	int i = 0, n = 0, tam = 0;
	char line_r[N_CHAR_MAX];
	char line_w[N_CHAR_MAX];
	char c[2];
	char num_pairs[N_CHAR_MAX];
	char* k = NULL;

	//Puts variables equal to 0.
	memset(line_r, 0, sizeof(line_r));
	memset(line_w, 0, sizeof(line_w));
   	memset(num_pairs, 0, sizeof(num_pairs));
	memset(c, 0, sizeof(c));

	lseek(filed[shardId], 0L, SEEK_SET); //Points to que beginning of the file.

	while (read(filed[shardId], c, 1) > 0) { //Reads the number of pairs already present in the file.
		if(strcmp(c,SPACE) == 0) {
			break;
		}
		strcat(num_pairs, c);
	}
	n = atoi(num_pairs);
	n--; //Decreases the number of pairs.
	sprintf(num_pairs, "%d", n);
	
	tam = strlen(num_pairs);
	for(i = tam; i < N_CHAR_MAX-2; i++) {
		strcat(num_pairs, SPACE);
	}

	strcat(num_pairs, END_LINE);
	lseek(filed[shardId], 0L, SEEK_SET); //Points to que beginning of the file.
	write(filed[shardId], num_pairs, strlen(num_pairs)); //Writes the new number of pairs.
	
	while (read(filed[shardId], c, 1) > 0) { //Reads file.
		if(strcmp(c,END_LINE) != 0) {		
			strcat(line_r, c); //Saves the "line" in each line of the file, except the first.
		} else {
			k = strtok(line_r, "<,>\n"); //Reads the value of the "key" from the line read.
			if(strcmp(key, k) == 0) { //Checks if it is the key to delete.
				for(i = 0; i < N_CHAR_MAX-2; i++) { //Replaces the content of the line that will be written by "_".
					strcat(line_w, SPACE);
				}
				strcat(line_w, END_LINE);
				lseek(filed[shardId], -N_CHAR_MAX+1, SEEK_CUR); //Points to the beginning of the line to delete.
				write(filed[shardId], line_w, strlen(line_w)); //"Deletes" line.
				return 0;
			} else {
				memset(line_r, 0, sizeof(line_r)); //Deletes the content of the variable that will save the line.
			}
		}
	}

	return 0;
}

int compacts_file(int shardId) {

	int i = 0;
	int n_elem = 0;
	int p_shard = 0;
	char line[N_CHAR_MAX];
	char fname[16];
	list_t* list  =NULL;

	memset(line, 0, sizeof(line));

	for(p_shard = 0; p_shard < HT_SIZE; p_shard++) { //Counts the number of elements of the shard.

		n_elem += lst_count(hash_t[shardId][p_shard]);
	}

	sprintf(line, "%d", n_elem);
	
	for(i = strlen(line); i < N_CHAR_MAX-2; i++) {
		strcat(line,SPACE);
	}

	strcat(line,END_LINE);

	close(filed[shardId]); //Closes the file.
	sprintf(fname, "fShard%d", shardId); //Name of the file.
	filed[shardId] = open(fname, O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR); //Deletes the existing file.

	lseek(filed[shardId], 0L, SEEK_SET); //Points to que beginning of the file.
	write(filed[shardId], line, strlen(line));	//Writes the number of elements.

	//Goes through all the positions of the shard.
	for(p_shard = 0; p_shard < HT_SIZE; p_shard++){
		i = 0;
		list = hash_t[shardId][p_shard];

		//Goes through the list 'list' of the position 'p_shard' of the shard and saves it's elements.
		while(lst_getPair(list, i) != NULL) {

			memset(line, 0, sizeof(line)); //Deletes the line that will be written.

			//Puts the pair in the variable "line".
			strcat(line, "<");
			strcat(line, (lst_getPair(list, i))->key);
			strcat(line, ",");
			strcat(line, (lst_getPair(list, i))->value);
			strcat(line, ">");
			for(i = strlen(line); i < N_CHAR_MAX-2; i++) {
				strcat(line,SPACE);
			}
			strcat(line,END_LINE);

			write(filed[shardId], line, strlen(line)); //Writes the element in the file.
			i++; //Next element.
		}
	}

	return 0;
}
