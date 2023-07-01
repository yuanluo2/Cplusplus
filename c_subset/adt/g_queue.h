/**
* @file      g_queue.h
* @brief     A queue implementation in pure C89, based on the bidirectional linked list.
* @author    Michael Ge
* @date      2023.05.29
* @version   v0.0.0
*/
#ifndef __G_ADT_QUEUE_H__
#define __G_ADT_QUEUE_H__

#include <stdlib.h>
#include <stddef.h>

typedef struct g_queue_node_t  g_queue_node_t;
typedef struct g_queue_t       g_queue_t;

struct g_queue_node_t {
	g_queue_node_t* prev;
	g_queue_node_t* next;
};

struct g_queue_t {
	g_queue_node_t* head;
	g_queue_node_t* tail;
	size_t elemSize;
	size_t size;
};

/**
* @define
* @desc get the queue's head.
* @param a pointer to a queue object. must not be NULL.
*/
#define g_queue_front(queue) \
	((void*)((queue)->head + 1))

/**
* @define
* @desc get the queue's tail.
* @param a pointer to a queue object. must not be NULL.
*/
#define g_queue_back(queue) \
	((void*)((queue)->tail + 1))

/**
* @define
* @desc get the queue's length.
* @param a pointer to a queue object. must not be NULL.
*/
#define g_queue_len(queue) \
	((queue)->size)
	
/**
* @define
* @desc check if the queue is empty.
* @param a pointer to a queue object. must not be NULL.
*/
#define g_queue_is_empty(queue) \
	((queue)->size == 0)

/**
* @desc create a queue.
* @param single element size, like sizeof(int), sizeof(char).
* @return a pointer to an queue object. return NULL if out of memory.
*/
g_queue_t*
g_create_queue(size_t elemSize){
	g_queue_t* queue = malloc(sizeof(g_queue_t));
	if (queue == NULL){
		return NULL;
	}
	
	queue->head = queue->tail = NULL;
	queue->elemSize = elemSize;
	queue->size = 0;
	return queue;
}

/**
* @desc destroy a queue.
* @param a pointer to a queue object. must not be NULL.
*/
void
g_destroy_queue(g_queue_t* queue){
	g_queue_node_t* node = queue->head;
	g_queue_node_t* next = NULL;
	
	while (node != NULL){
		next = node->next;
		free(node);
		node = next;
	}
	
	free(queue);
}

/**
* @desc queue add to tail. usage is similar to malloc().
* @param a pointer to a queue object. must not be NULL.
* @return a pointer to the address of an allocated memory block. return NULL if out of memory.
*/
void*
g_queue_push_back(g_queue_t* queue){
	g_queue_node_t* node = malloc(sizeof(g_queue_node_t) + queue->elemSize);
	if (node == NULL){
		return NULL;
	}
	
	if (queue->size == 0){
		node->prev = node->next = NULL;
		queue->head = queue->tail = node;
	}
	else{
		node->next = NULL;
		node->prev = queue->tail;
		queue->tail->next = node;
		queue->tail = node;
	};
	
	++(queue->size);
	return node + 1;
}

/**
* @desc queue add to tail. usage is similar to malloc().
* @param a pointer to a queue object. must not be NULL.
* @return a pointer to the address of an allocated memory block. return NULL if out of memory.
*/
void*
g_queue_push_front(g_queue_t* queue){
	g_queue_node_t* node = malloc(sizeof(g_queue_node_t) + queue->elemSize);
	if (node == NULL){
		return NULL;
	}
	
	if (queue->size == 0){
		node->prev = node->next = NULL;
		queue->head = queue->tail = node;
	}
	else{
		node->next = queue->head;
		node->prev = NULL;
		queue->head->prev = node;
		queue->head = node;
	};
	
	++(queue->size);
	return node + 1;
}

/**
* @desc queue pop tail.
* @param a pointer to a queue object. must not be NULL.
*/
void
g_queue_pop_back(g_queue_t* queue){
	if (queue->size == 0){
		return;
	}
	
	if (queue->size == 1){
		free(queue->tail);
		queue->head = queue->tail = NULL;
	}
	else{
		g_queue_node_t* tail_prev = queue->tail->prev;
		tail_prev->next = NULL;
		free(queue->tail);
		queue->tail = tail_prev;
	}
	
	--(queue->size);
}

/**
* @desc queue pop head.
* @param a pointer to a queue object. must not be NULL.
*/
void
g_queue_pop_front(g_queue_t* queue){
	if (queue->size == 0){
		return;
	}
	
	if (queue->size == 1){
		free(queue->tail);
		queue->head = queue->tail = NULL;
	}
	else{
		g_queue_node_t* head_next = queue->head->next;
		head_next->prev = NULL;
		free(queue->head);
		queue->head = head_next;
	}
	
	--(queue->size);
}

/*
	usage:
	
	#include <stdio.h>
	#include "g_queue.h"

	int main(){
		g_queue_t* queue = g_create_queue(sizeof(int));
		if (queue == NULL){
			fprintf(stderr, "out of memory.");
			exit(EXIT_FAILURE);
		}
		
		int i;
		int* elem;
		for (i = 0;i < 9;++i){
			elem = g_queue_push_back(queue);
			if (elem == NULL){
				fprintf(stderr, "out of memory.");
				exit(EXIT_FAILURE);
			}
			
			*elem = i * i;
		}
		
		int len = g_queue_len(queue);
		for (i = 0;i < len;++i){
			elem = g_queue_front(queue);
			printf("%d\n", *elem);
			g_queue_pop_front(queue);
		}
		
		g_destroy_queue(queue);
	}
*/

#endif
