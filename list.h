#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

typedef struct list {
	struct list *prev, *next;
	void* val;
}* list_t;

// clears the list and deallocates memory
void list_clear(list_t* l);

// creates new node in the list
// ! moves head of the list to newly created node !
int list_push(list_t* l);

// pops an element from the list
void list_pop(list_t* l);

#endif
