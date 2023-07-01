/**
* @file      g_array.h
* @brief     A dynamic array implementation in pure C89.
* @author    Michael Ge
* @date      2023.05.28
* @version   v0.0.0
*/
#ifndef __G_ADT_ARRAY_H__
#define __G_ADT_ARRAY_H__

#include <stdlib.h>
#include <stddef.h>

/**
* @define 
* @desc default capacity. must be a integer.
*/
#define g_DEFAULT_ARRAY_CAPACITY 5

/**
* @define 
* @desc the array's capacity will multiply this value when expand. must be a integer.
*/
#define g_DEFAULT_ARRAY_EXPAND_FACTOR 2

/**
* @struct
* @desc array object.
*/
typedef struct g_array_t g_array_t;

struct g_array_t {
	size_t size;
	size_t capacity;
	size_t elemSize;
};

/**
* @desc create a array, and set the default capacity by the param.
* @param[capacity] given capacity.
* @param[elemSize] single element size, like sizeof(int), sizeof(char).
* @return a pointer to an array object. return NULL if out of memory.
*/
g_array_t*
g_create_array_n(size_t capacity, size_t elemSize){
	g_array_t* arr = malloc(sizeof(g_array_t) + capacity * elemSize);
	if (arr == NULL){
		return NULL;
	}
	
	arr->size = 0;
	arr->capacity = capacity;
	arr->elemSize = elemSize;
	return arr;
}

/**
* @define
* @desc create a array, the default capacity is set to g_DEFAULT_ARRAY_CAPACITY.
* @param single element size, like sizeof(int), sizeof(char).
*/
#define g_create_array(elemSize) \
	g_create_array_n(g_DEFAULT_ARRAY_CAPACITY, elemSize)

/**
* @define
* @desc destroy a array.
* @param a pointer to an array object. must not be NULL.
*/
#define g_destroy_array(arr) \
	do {                     \
		free((arr));         \
	} while(0)

/**
* @define
* @desc get the starting address position of the specified index in the array.
* @param[arr] a pointer to an array object. must not be NULL.
* @param[index] 
*/
#define g_array_get(arr, index) \
	((void*)((char*)((arr) + 1) + (arr)->elemSize * index))
	
/**
* @define
* @desc get the array's length.
* @param a pointer to an array object. must not be NULL.
*/
#define g_array_len(arr) \
	((arr)->size)

/**
* @define
* @desc get the array's capacity.
* @param a pointer to an array object. must not be NULL.
*/	
#define g_array_capacity(arr) \
	((arr)->capacity)

/**
* @define
* @desc add an element into the array. usage is similar to malloc().
* @param a secondary pointer to an array object. must not be NULL.
* @return a pointer to the address of an allocated memory block. return NULL if out of memory.
*/
void* 
g_array_add_back(g_array_t** arr){
	if ((*arr)->size == (*arr)->capacity){
		g_array_t* temp = realloc(*arr, sizeof(g_array_t) + g_DEFAULT_ARRAY_EXPAND_FACTOR * ((*arr)->capacity) * ((*arr)->elemSize));
		if (temp == NULL){
			return NULL;
		}
		
		*arr = temp;
		(*arr)->capacity *= g_DEFAULT_ARRAY_EXPAND_FACTOR;
	}
	
	return ((char*)(*arr + 1)) + ((*arr)->elemSize) * (((*arr)->size)++);
}

/*
	usage:

	#include <stdio.h>
	#include "g_array.h"

	int main(){
		g_array_t* arr;
		int* elem;
		int i;
		int len;
		
		arr = g_create_array(sizeof(int));
		if (arr == NULL){
			fprintf(stderr, "out of memory.");
			exit(EXIT_FAILURE);
		}
		
		for (i = 0;i < 9;++i){
			elem = g_array_add_back(&arr);
			if (elem == NULL){
				fprintf(stderr, "out of memory.");
				exit(EXIT_FAILURE);
			}
			
			*elem = i * i;
		}
		
		len = g_array_len(arr);
		for (i = 0;i < len;++i){
			elem = g_array_get(arr, i);
			printf("%d\n", *elem);
		}
		
		g_destroy_array(arr);
	}
*/
#endif
