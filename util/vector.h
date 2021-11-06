#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


// Vector is an automatic-resizing array.
typedef struct {
	void *    data;       // actual data buffer
	uintptr_t size, cap;  // size of the data and buffer, in bytes
	uintptr_t objectSize; // size of the object
} vector_Vector;

// Create allocates a new vector.
vector_Vector *vector_Create(uintptr_t objectSize);

// Destroy properly frees all data related to the structure, and itself.
void vector_Destroy(vector_Vector *vec);

// Push pushes an object at the back of the vector.
//
// If data is NULL, the data is zeroed.
//
// Returns a pointer to data.
void *vector_Push(vector_Vector *vec, const void *data);

// Append pushes multiple objects at the back of the buffer.
//
// If data is NULL, the data is zeroed.
void vector_Append(vector_Vector *vec, const void *data, uintptr_t n);

// Resize resizes the vector to a new number of elements.
void vector_Resize(vector_Vector *vec, uintptr_t size);

// Size returns the number of objects in a vector.
uintptr_t vector_Size(vector_Vector *vec);

// Clear clears a vector, without resizing the buffer.
void vector_Clear(vector_Vector *vec);

// ShrinkBuffer shrinks the internal buffer to exactly the size of the elements.
void vector_ShrinkBuffer(vector_Vector *vec);

// Capacity returns the size of the internal buffer.
uintptr_t vector_Capacity(vector_Vector *vec);

// Reserve reallocates the size of the buffer to CAP objects,
// if the current buffer is smaller.
void vector_Reserve(vector_Vector *vec, uintptr_t cap);

// At returns the i-th object of the vector.
//
// No boundary test is performed.
void *vector_At(vector_Vector *vec, uintptr_t i);

// Data returns the data buffer.
void *vector_Data(vector_Vector *vec);


#ifdef __cplusplus
}
#endif
