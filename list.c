#include "list.h"

void list_clear(list_t *l){
	list_t p = *l;
	while(p != NULL){
		list_t temp = p->next;
		if(p->val != NULL)
			free(p->val);
		free(p);
		p = temp;
	}
	*l = NULL;
}

int list_push(list_t *l){
	if(*l == NULL){
		*l = (list_t)malloc(sizeof(struct list));
		if(*l == NULL) return 1;
		(*l)->val = NULL;
		(*l)->next = NULL;
		(*l)->prev = NULL;
	}
	else {
		list_t prev = *l;

		*l = (list_t)malloc(sizeof(struct list));
		if(*l == NULL) return 1;

		(*l)->val = NULL;
		prev->next = *l;
		(*l)->prev = prev;
		(*l)->next = NULL;
	}
	return 0;
}

void list_pop(list_t *l){
	if(*l != NULL){
		list_t temp = (*l)->prev;
		free(*l);
		*l = temp;
		if(*l != NULL)
			(*l)->next = NULL;
	}
}
