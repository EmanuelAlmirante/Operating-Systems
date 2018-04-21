/*
 * list.h - definitions and declarations of the KV_t list
 */

#include "kos_client.h"

/*lst_iitem - each element of the list points to the next element*/
typedef struct lst_iitem {
   KV_t* pair;
   struct lst_iitem *next;
} node;

/*list_t*/
typedef struct {
   node* first;
} list_t;

/*lst_new - allocates memory for list_t and initializes it*/
list_t* lst_new();

/*lst_search - searches an item with the key 'key' in the list 'list'.
   Returns the KV_t pair if it exists or NULL if it doesn't*/
KV_t* lst_search(list_t* list, char* key);

/*lst_insert - insert a new item with value 'value' in list 'list'*/
void lst_insert(list_t* list, char* key, char* value);

/*lst_remove - remove first item of value 'value' from list 'list'*/
void lst_remove(list_t* list, char* key);

/*lst_getPair - devolve o par na posicao 'p' da lista 'list'*/
KV_t* lst_getPair(list_t* list, int p);

/*lst_count - conta o numero de elementos presentes na lista 'list'*/
int lst_count(list_t* list);
