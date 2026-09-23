
/*
 *   Source File [rose_array.c]
 */

#include "rose.h"

ROSE_Array* ROSE_ArrayCreate(usize stride) {
	ROSE_Array* array = malloc(sizeof(ROSE_Array));
	assert(array);
	array->data = malloc(stride);
	array->stride = stride;
	array->size = 0;
	array->capacity = 1;
	assert(array->data);
	return array;
}

void ROSE_ArrayPushback(ROSE_Array* array, void* element) {
	if (array->size >= array->capacity) {
		usize new_capacity = array->capacity * 2;
		void* new_data = malloc(new_capacity * array->stride);
		assert(new_data);
		memcpy(new_data, array->data, array->capacity * array->stride);
		free(array->data);
		array->capacity = new_capacity;
		array->data = new_data;
	}

	memcpy(array->data + (array->stride * array->size), element, array->stride);
	array->size++;
}

void ROSE_ArrayDestroy(ROSE_Array* array) {
	if (!array) { return; }
	free(array->data);
	free(array);
}

