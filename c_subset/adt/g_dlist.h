/**
* @file      g_dlist.h
* @brief     A bidirectional linked list implementation in pure C89.
* @author    Michael Ge
* @date      2023.05.28
* @version   v0.0.0
*/
#ifndef __G_ADT_DLIST_H__
#define __G_ADT_DLIST_H__

#include <stdlib.h>
#include <stddef.h>

typedef struct g_dlist_node_t  g_dlist_node_t;
typedef struct g_dlist_t       g_dlist_t;

struct g_dlist_node_t {
	g_dlist_node_t* prev;
	g_dlist_node_t* next;
};

struct g_dlist_t {
	g_dlist_node_t* head;
	g_dlist_node_t* tail;
	size_t elemSize;
	size_t size;
};

/**
* @desc create a dlist.
* @param single element size, like sizeof(int), sizeof(char).
* @return a pointer to an dlist object. return NULL if out of memory.
*/
g_dlist_t*
g_create_dlist(size_t elemSize){
	g_dlist_t* dlist = malloc(sizeof(g_dlist_t));
	if (dlist == NULL){
		return NULL;
	}
	
	dlist->head = dlist->tail = NULL;
	dlist->elemSize = elemSize;
	dlist->size = 0;
	return dlist;
}

/**
* @desc destroy a dlist.
* @param a pointer to a dlist object. must not be NULL.
*/
void
g_destroy_dlist(g_dlist_t* dlist){
	g_dlist_node_t* node = dlist->head;
	g_dlist_node_t* next = NULL;
	
	while (node != NULL){
		next = node->next;
		free(node);
		node = next;
	}
	
	free(dlist);
}

/**
* @desc dlist add to tail. usage is similar to malloc().
* @param a pointer to a dlist object. must not be NULL.
* @return a pointer to the address of an allocated memory block. return NULL if out of memory.
*/
void*
g_dlist_add_back(g_dlist_t* dlist){
	g_dlist_node_t* node = malloc(sizeof(g_dlist_node_t) + dlist->elemSize);
	if (node == NULL){
		return NULL;
	}
	
	node->next = NULL;
	if (dlist->head == NULL){
		dlist->head = dlist->tail = node;
		dlist->head->prev = NULL;
	}
	else {
		node->prev = dlist->tail;
		dlist->tail->next = node;
		dlist->tail = node;
	}
	
	++(dlist->size);
	return node + 1;
}

/**
* @desc dlist add to head. usage is similar to malloc().
* @param a pointer to a dlist object. must not be NULL.
* @return a pointer to the address of an allocated memory block. return NULL if out of memory.
*/
void*
g_dlist_add_front(g_dlist_t* dlist){
	g_dlist_node_t* node = malloc(sizeof(g_dlist_node_t) + dlist->elemSize);
	if (node == NULL){
		return NULL;
	}
	
	if (dlist->head == NULL){
		node->next = NULL;
		dlist->head = dlist->tail = node;
		dlist->head->prev = NULL;
	}
	else {
		node->prev = NULL;
		node->next = dlist->head;
		dlist->head->prev = node;
		dlist->head = node;
	}
	
	++(dlist->size);
	return node + 1;
}

/**
* @define
* @desc get the dlist's head.
* @param a pointer to a dlist object. must not be NULL.
*/
#define g_dlist_front(dlist) \
	((void*)((dlist)->head + 1))

/**
* @define
* @desc get the dlist's tail.
* @param a pointer to a dlist object. must not be NULL.
*/
#define g_dlist_back(dlist) \
	((void*)((dlist)->tail + 1))

/**
* @define
* @desc get the dlist's length.
* @param a pointer to a dlist object. must not be NULL.
*/
#define g_dlist_len(dlist) \
	((dlist)->size)

/**
* @define
* @desc get the given pointer's header part.
* @param a pointer allocated by g_dlist_add_back() or g_dlist_add_front(). must not be NULL.
*/	
#define g_dlist_get_header(elem) \
	((g_dlist_node_t*)(((char*)(elem)) - sizeof(g_dlist_node_t)))

/**
* @define
* @desc get the given pointer's next.
* @param a pointer allocated by g_dlist_add_back() or g_dlist_add_front(). must not be NULL.
*/
#define g_dlist_next(elem) \
	((g_dlist_get_header(elem)->next) == NULL ? NULL : ((void*)(g_dlist_get_header(elem)->next + 1)))

/**
* @define
* @desc get the given pointer's prev.
* @param a pointer allocated by g_dlist_add_back() or g_dlist_add_front(). must not be NULL.
*/
#define g_dlist_prev(elem) \
	((g_dlist_get_header(elem)->prev) == NULL ? NULL : ((void*)(g_dlist_get_header(elem)->prev + 1)))

/**
* @desc dlist remove.
* @param[dlist] a pointer to a dlist object. must not be NULL.
* @param[elem] a pointer allocated by g_dlist_add_back() or g_dlist_add_front(). must not be NULL.
* @return a pointer to the next node.
*/
void*
g_dlist_remove(g_dlist_t* dlist, void* elem){
	g_dlist_node_t* node = g_dlist_get_header(elem);
	void* res = NULL;
	
	if (node->prev != NULL && node->next != NULL){
		res = (void*)(node->next + 1);
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}
	else if (node->prev != NULL && node->next == NULL){
		res = NULL;
		node->prev->next = NULL;
		dlist->tail = node->prev;
	}
	else if (node->prev == NULL && node->next != NULL){
		res = (void*)(node->next + 1);
		node->next->prev = NULL;
		dlist->head = node->next;
	}
	else{
		res = NULL;
		dlist->head = dlist->tail = NULL;
	}
	
	free(node);
	--(dlist->size);
	return res;
}

/*
	usage:
	
	#include <stdio.h>
	#include "g_dlist.h"

	int main(){
		g_dlist_t* dlist = g_create_dlist(sizeof(int));
		if (dlist == NULL){
			fprintf(stderr, "out of memory.");
			exit(EXIT_FAILURE);
		}
		
		int i;
		int* elem;
		for (i = 0;i < 9;++i){
			elem = g_dlist_add_front(dlist);
			if (elem == NULL){
				fprintf(stderr, "out of memory.");
				exit(EXIT_FAILURE);
			}
			
			*elem = i * i;
		}

		elem = g_dlist_front(dlist);
		while (elem != NULL){
			if (*elem % 2 == 0){
				elem = g_dlist_remove(dlist, elem);
			}
			else{
				elem = g_dlist_next(elem);
			}
		}

		elem = g_dlist_front(dlist);
		while (elem != NULL){
			printf("%d\n", *elem);
			elem = g_dlist_next(elem);
		}
		
		g_destroy_dlist(dlist);
	}
*/
#endif
