#pragma once

#include <stdint.h>
#include <string.h>
#include "../util/vector.h"

namespace helos {
namespace runtime {


// Vector is a dynamic sized array.
//
// Right now it simply wraps the C utility vector.
// It can only hold POD (Plain Old Data).
template<typename Type>
class Vector {
public:
	// Default constructor.
	Vector() { vec = vector_Create(sizeof(Type)); }
	// Copy construct from a C-style array.
	Vector(const Type *arr, uintptr_t n) {
		vec = vector_Create(sizeof(Type));
		vector_Resize(vec, n);
		memcpy(vector_Data(vec), arr);
	}
	// Copy construct from another Vector object.
	Vector(const Vector &other) {
		vec = vector_Create(sizeof(Type));
		vector_Resize(vec, other.Size());
		memcpy(vector_Data(vec), other.Data());
	}
	// Move construct from another Vector object.
	Vector(Vector &&move): vec(move.vec) {
		move.vec = vector_Create(sizeof(Type));
	}

	// Deconstructor.
	~Vector() { vector_Destroy(vec); }

public:
	// Pushes one object to the back of the vector.
	//
	// If data is NULL, the data is zeroed.
	//
	// Returns an reference to the object.
	Type &Push(const Type &data) { return *((Type *)vector_Push(vec, &data)); }

	// Appends multiple objects to the back of the vector.
	//
	// If data is NULL, the data is zeroed.
	void Append(const Type *data, uintptr_t n) { vector_Append(vec, data, n); }

	// Resize resizes the vector to a new number of elements.
	void Resize(uintptr_t newsize) { vector_Resize(vec, newsize); }

	// Size returns the size of the vector in objects.
	uintptr_t Size() { return vector_Size(vec); }

	// Capacity returns the size of the internal buffer.
	uintptr_t Capacity() { return vector_Capacity(vec); }

	// Clear clears a vector, without resizing the buffer.
	void Clear() { vector_Clear(vec); }

	// ShrinkBuffer shrinks the internal buffer to exactly the size of the elements.
	void ShrinkBuffer() { vector_ShrinkBuffer(vec); }

	// Reserve reallocates the size of the buffer to CAP objects,
	// if the current buffer is smaller.
	void Reserve(uintptr_t cap) { vector_Reserve(vec, cap); }

public:
	// Reference operators.
	Type       &operator[](size_t at) { return *((Type *)vector_At(vec, at)); }
	const Type &operator[](size_t at) const { return *((Type *)vector_At(vec, at)); }

	// Data accesses the underlying array.
	Type *Data() { return vector_Data(vec); }

private:
	vector_Vector *vec;
};


} // namespace runtime
} // namespace helos
