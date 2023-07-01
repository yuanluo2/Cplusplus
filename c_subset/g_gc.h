/**
* @file      g_gc.h
* @brief     A simple mark-sweep garbage collector implemented in C89.
* @details   A garbage collector with mark-sweep algorithm, writes in pure C to help me get out of the memory problems.
* @author    Michael Ge
* @date      2023.05.28
* @version   v0.0.0
*/
#ifndef __G_GARBAGE_COLLECTOR_H__
#define __G_GARBAGE_COLLECTOR_H__

#include <stdlib.h>
#include <stddef.h>

typedef struct g_gc_node_t  g_gc_node_t;
typedef struct g_gc_t       g_gc_t;

struct g_gc_node_t {
	g_gc_node_t*   prev;
	g_gc_node_t*   next;
	unsigned char  marked;
};

struct g_gc_t {
	g_gc_node_t* head;
};

/**
* @desc create a garbage collector.
* @return a initialized g_gc_t object. return NULL if out of memory.
*/
g_gc_t* 
g_create_gc(){
	g_gc_t* gc = malloc(sizeof(g_gc_t));
	if (gc == NULL){
		return NULL;
	}
	
	gc->head = NULL;
	return gc;
}

/**
* @desc destroy a garbage collector and all allocated memory.
* @param a valid g_gc_t object, make sure this param is not NULL.
*/
void
g_destroy_gc(g_gc_t* gc){
	g_gc_node_t* node = gc->head;
	g_gc_node_t* next = NULL;
	
	while (node != NULL){
		next = node->next;
		free(node);
		node = next;
	}
	
	free(gc);
}

/**
* @define 
* @desc a macro to get the given pointer's header(a pointer to a g_gc_node_t object).
* @param a pointer created by the function g_gc_malloc() or g_gc_mark_malloc().
*/
#define g_gc_elem_header(elem) \
	((g_gc_node_t*)((char*)(elem) - sizeof(g_gc_node_t)))

/**
* @define
* @desc a macro to mark the give pointer, when the pointer is marked, gc won't collect it.
* @param a pointer created by the function g_gc_malloc() or g_gc_mark_malloc().
*/
#define g_gc_mark(elem)                      \
	do {                                     \
		g_gc_elem_header(elem)->marked = 1;  \
	} while(0)

/**
* @define
* @desc a macro to unmark the give pointer.
* @param a pointer created by the function g_gc_malloc() or g_gc_mark_malloc().
*/
#define g_gc_unmark(elem)                    \
	do {                                     \
		g_gc_elem_header(elem)->marked = 0;  \
	} while(0)

/**
* @desc doing a gc collect.
* @param a valid g_gc_t object, make sure this param is not NULL.
*/
void
g_do_gc(g_gc_t* gc){	
	g_gc_node_t* node = gc->head;
	
	while (node != NULL){
		if (node->marked == 1){
			node = node->next;
		}
		else{
			if (node->prev != NULL && node->next != NULL){
				g_gc_node_t* current_next = node->next;
				node->prev->next = node->next;
				node->next->prev = node->prev;
				free(node);
				node = current_next;
			}
			else if (node->prev != NULL && node->next == NULL){
				node->prev->next = NULL;
				free(node);
				return;
			}
			else if (node->next != NULL && node->prev == NULL){
				gc->head = node->next;
				gc->head->prev = NULL;
				free(node);
				node = gc->head;
			}
			else{
				gc->head = NULL;
				free(node);
				return;
			}
		}
	}
}

/**
* @desc similar to malloc(), but it will do gc collect when there is no enough memory. this function set the marked flag to 0 by default.
* @param[gc] a valid g_gc_t object, make sure this param is not NULL.
* @param[elemSize] block size.
* @return a block of memory. return NULL if out of memory.
*/
void*
g_gc_malloc(g_gc_t* gc, size_t elemSize){
	g_gc_node_t* node = malloc(sizeof(g_gc_node_t) + elemSize);
	if (node == NULL){
		g_do_gc(gc);
		
		node = malloc(sizeof(g_gc_node_t) + elemSize);
		if (node == NULL){
			return NULL;
		}
	}
	
	node->marked = 0;
	if (gc->head == NULL){
		node->next = node->prev = NULL;
		gc->head = node;
	}
	else{
		node->prev = NULL;
		node->next = gc->head;
		gc->head->prev = node;
		gc->head = node;
	}
	
	return node + 1;
}

/**
* @desc similar to g_gc_malloc(), but set the marked flag to 1 by default.
* @param[gc] a valid g_gc_t object, make sure this param is not NULL.
* @param[elemSize] block size.
* @return a block of memory. return NULL if out of memory.
*/
void*
g_gc_marked_malloc(g_gc_t* gc, size_t elemSize){
	void* elem = g_gc_malloc(gc, elemSize);
	if (elem == NULL){
		return NULL;
	}
	
	g_gc_mark(elem);
	return elem;
}

#endif
