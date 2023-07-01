/**
* @file      g_stack.h
* @brief     A stack implementation in pure C89.
* @detail    This stack's design is based on singly linked list
* @author    Michael Ge
* @date      2023.05.28
* @version   v0.0.0
*/
#ifndef __G_ADT_STACK_H__
#define __G_ADT_STACK_H__

#include <stdlib.h>
#include <stddef.h>

typedef struct g_stack_node_t  g_stack_node_t;
typedef struct g_stack_t       g_stack_t;

struct g_stack_node_t {
	g_stack_node_t* next;
};

struct g_stack_t {
	g_stack_node_t* head;
	size_t elemSize;
	size_t size;
};

/**
* @desc create a stack.
* @param single element size, like sizeof(int), sizeof(char).
* @return a pointer to a stack object. return NULL if out of memory.
*/
g_stack_t*
g_create_stack(size_t elemSize){
	g_stack_t* stack = malloc(sizeof(g_stack_t));
	if (stack == NULL){
		return NULL;
	}
	
	stack->size = 0;
	stack->head = NULL;
	stack->elemSize = elemSize;
	return stack;
}

/**
* @desc destroy a stack.
* @param a pointer to a stack object. must not be NULL.
*/
void
g_destroy_stack(g_stack_t* stack){
	g_stack_node_t* node = stack->head;
	g_stack_node_t* next = NULL;
	
	while (node != NULL){
		next = node->next;
		free(node);
		node = next;
	}
	
	free(stack);
}

/**
* @desc stack push. usage is similar to malloc().
* @param a pointer to a stack object. must not be NULL.
* @return a pointer to the address of an allocated memory block. return NULL if out of memory.
*/
void*
g_stack_push(g_stack_t* stack){
	g_stack_node_t* node = malloc(sizeof(g_stack_node_t) + stack->elemSize);
	if (node == NULL){
		return NULL;
	}
	
	node->next = stack->head;
	stack->head = node;
	++(stack->size);
	return node + 1;
}

/**
* @define
* @desc get the stack's length.
* @param a pointer to a stack object. must not be NULL.
*/
#define g_stack_len(stack) \
	((stack)->size)

/**
* @define
* @desc check if the stack is empty.
* @param a pointer to a stack object. must not be NULL.
*/
#define g_stack_is_empty(stack) \
	(g_stack_len(stack) == 0)

/**
* @define
* @desc get the pointer to the stack's top element.
* @param a pointer to a stack object. must not be NULL.
*/
#define g_stack_top(stack) \
	((void*)((stack)->head + 1))

/**
* @desc stack pop.
* @param a pointer to a stack object. must not be NULL.
*/
void
g_stack_pop(g_stack_t* stack){
	if (g_stack_is_empty(stack)){
		return;
	}
	
	g_stack_node_t* next = stack->head->next;
	free(stack->head);
	stack->head = next;
	--(stack->size);
}

/*
	usage:
	
	#include <stdio.h>
	#include "g_stack.h"

	int main(){
		g_stack_t* stack = g_create_stack(sizeof(int));
		if (stack == NULL){
			fprintf(stderr, "out of memory.\n");
			exit(EXIT_FAILURE);
		}
		
		int* elem;
		int i;
		for (i = 0;i < 13;++i){
			elem = g_stack_push(stack);
			if (elem == NULL){
				fprintf(stderr, "out of memory.\n");
				exit(EXIT_FAILURE);
			}
			
			*elem = i * i;
		}
		
		while (!g_stack_is_empty(stack)){
			elem = g_stack_top(stack);
			printf("%d\n", *elem);
			g_stack_pop(stack);
		}
		
		g_destroy_stack(stack);
	}
*/
#endif
