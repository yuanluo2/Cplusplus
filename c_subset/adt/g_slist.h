/**
* @file      g_slist.h
* @brief     A singly linked list implementation in pure C89.
* @author    Michael Ge
* @date      2023.05.28
* @version   v0.0.0
*/
#ifndef __G_ADT_SLIST_H__
#define __G_ADT_SLIST_H__

#include <stdlib.h>
#include <stddef.h>

typedef struct g_slist_node_t  g_slist_node_t;
typedef struct g_slist_t       g_slist_t;

struct g_slist_node_t{
	g_slist_node_t* next;
};

struct g_slist_t {
	g_slist_node_t* head;
	g_slist_node_t* tail;
	size_t elemSize;
	size_t size;
};

/**
* @desc create a slist.
* @param single element size, like sizeof(int), sizeof(char).
* @return a pointer to an slist object. return NULL if out of memory.
*/
g_slist_t*
g_create_slist(size_t elemSize) {
	g_slist_t* slist = malloc(sizeof(g_slist_t));
	if (slist == NULL){
		return NULL;
	}
	
	slist->head = slist->tail = NULL;
	slist->elemSize = elemSize;
	slist->size = 0;
	return slist;
}

/**
* @desc destroy a slist.
* @param a pointer to a slist object. must not be NULL.
*/
void
g_destroy_slist(g_slist_t* slist){
	g_slist_node_t* node = slist->head;
	g_slist_node_t* next = NULL;
	
	while (node != NULL){
		next = node->next;
		free(node);
		node = next;
	}
	
	free(slist);
}

/**
* @desc slist add to tail. usage is similar to malloc().
* @param a pointer to a slist object. must not be NULL.
* @return a pointer to the address of an allocated memory block. return NULL if out of memory.
*/
void*
g_slist_add_back(g_slist_t* slist){
	g_slist_node_t* node = malloc(sizeof(g_slist_node_t) + slist->elemSize);
	if (node == NULL){
		return NULL;
	}
	
	node->next = NULL;
	if (slist->head == NULL){
		slist->head = slist->tail = node;
	}
	else{
		slist->tail->next = node;
		slist->tail = node;
	}
	
	++(slist->size);
	return node + 1;
}

/**
* @desc slist add to head. usage is similar to malloc().
* @param a pointer to a slist object. must not be NULL.
* @return a pointer to the address of an allocated memory block. return NULL if out of memory.
*/
void*
g_slist_add_front(g_slist_t* slist){
	g_slist_node_t* node = malloc(sizeof(g_slist_node_t) + slist->elemSize);
	if (node == NULL){
		return NULL;
	}
	
	if (slist->head == NULL){
		node->next = NULL;
		slist->head = slist->tail = node;
	}
	else{
		node->next = slist->head;
		slist->head = node;
	}
	
	++(slist->size);
	return node + 1;
}

/**
* @define
* @desc get the slist's head.
* @param a pointer to a slist object. must not be NULL.
*/
#define g_slist_front(slist) \
	((void*)(((slist)->head) + 1))
	
/**
* @define
* @desc get the slist's tail.
* @param a pointer to a slist object. must not be NULL.
*/
#define g_slist_back(slist) \
	((void*)(((slist)->tail) + 1))
	
/**
* @define
* @desc get the slist's length.
* @param a pointer to a slist object. must not be NULL.
*/
#define g_slist_len(slist) \
	((slist)->size)

/**
* @define
* @desc get the given pointer's header part.
* @param a pointer allocated by g_slist_add_back() or g_slist_add_front(). must not be NULL.
*/
#define g_slist_get_header(elem) \
	((g_slist_node_t*)(((char*)(elem)) - sizeof(g_slist_node_t)))

/**
* @define
* @desc get the given pointer's next.
* @param a pointer allocated by g_slist_add_back() or g_slist_add_front(). must not be NULL.
*/
#define g_slist_next(elem) \
	((g_slist_get_header(elem)->next) == NULL ? NULL : ((void*)(g_slist_get_header(elem)->next + 1)))

/*
	usage:
	
	#include <stdio.h>
	#include "g_slist.h"

	int main(){
		g_slist_t* slist = g_create_slist(sizeof(int));
		if (slist == NULL){
			fprintf(stderr, "out of memory.");
			exit(EXIT_FAILURE);
		}
		
		int i;
		int* elem;
		for (i = 0;i < 9;++i){
			elem = g_slist_add_front(slist);
			if (elem == NULL){
				fprintf(stderr, "out of memory.");
				exit(EXIT_FAILURE);
			}
			
			*elem = i * i;
		}
		
		elem = g_slist_front(slist);
		while (elem != NULL){
			printf("%d\n", *elem);
			elem = g_slist_next(elem);
		}
		
		g_destroy_slist(slist);
	}
*/
#endif
