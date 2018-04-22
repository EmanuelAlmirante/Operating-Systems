/*
 * list.c - implementation of KV_t list functions 
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <kos_client.h>
#include <list.h>

list_t* lst_new() {
	list_t* list;
	list = (list_t*) malloc(sizeof(list_t));
	list->first = NULL;
	return list;
}

KV_t* lst_search(list_t* list, char* key) {
	node* link = list->first;
	while(link != NULL) {
		if(strcmp(link->pair->key, key) == 0)
			return link->pair;
		link = link->next;
	}
	return NULL;
}

void lst_insert(list_t* list,char* key, char* value) {

	KV_t* pair = (KV_t*)malloc(sizeof(KV_t));
	strcpy(pair->key, key);
	strcpy(pair->value, value);

	if (list-> first == NULL) {
		list->first = (node*)malloc(sizeof(node));
		list->first->pair = pair;
       		list->first->next = NULL;
    	}
	else {
	        node* link = list->first;
	
		while(link->next != NULL)
			link = link->next;

		link->next = (node*)malloc(sizeof(node));
    		link->next->pair = pair;
		link->next->next = NULL;
	}
	return;
} 

void lst_remove(list_t* list, char* key) {
	node* link = list->first;
	node* ant = NULL;
 	while(link != NULL) {
		if(strcmp(link->pair->key, key) ==0 ) {
			if(ant == NULL) {
				list->first = link->next;
				free(link);
				return;
			}
			else{
			 	ant -> next = link -> next;
		   		free(link);
				return;
			}
		}
		ant = link;
		link = link->next;
	}
	return;
}

KV_t* lst_getPair(list_t* list, int p) {
	int i = 0;
	node* link = list->first;

	while(link != NULL) {
		if(i == p){
			return link->pair;
		}
		i++;
		link = link->next;
	}

	return NULL;
}

int lst_count(list_t* list) {
	int c = 0;
	node* link = list->first;
	while(link != NULL) {
		link = link->next;
		c++;
	}
	return c;
}
