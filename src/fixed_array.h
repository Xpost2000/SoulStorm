#ifndef FIXED_ARRAY_H
#define FIXED_ARRAY_H

#include "common.h"
#include "memory_arena.h"

/*
 * This is more like a fixed_vector but too many
 * STL connotations
 *
 * This is a fixed size array that is supposed to be backed by a memory arena
 * since if you're using the plain heap you can just use std::vector to let go of
 * the intentional limitations...
 */

template<class T>
struct Fixed_Array {
    Fixed_Array() {
        data = nullptr;
        size = capacity = 0;
    }

    Fixed_Array(Memory_Arena* arena, T* arr, size_t length, size_t capacity = 0) {
        init_reserve_inplace(arena, arr, length, capacity);
    }

    Fixed_Array(Memory_Arena* arena, size_t capacity) {
        init_reserve(arena, capacity);
    }

    void init_reserve(Memory_Arena* arena, size_t capacity) {
        data = (T*)arena->push_unaligned(capacity * sizeof(*data));

        // placement new eh?
        {
            for (size_t i = 0; i < capacity; ++i) {
                new (data + i) T();
            }
        }

        size = 0;
        this->capacity = capacity;
    }

    void init_reserve_inplace(Memory_Arena* arena, T* arr, size_t length, size_t capacity = 0) {
        if (capacity == 0) capacity = length;
        init_reserve(arena, capacity);

        for (size_t i = 0; i < length; ++i) {
            data[i] = arr[i];
        }

        size = length;
    }

    T& operator[](int index) {
        if (index < 0) index = (size + index);
        // NOTE: I don't actually want to catch "invalid access". I want to catch "out of bounds" access.
        // I don't mind poking within memory in defined areas as long as it's within my yard.
        assertion(index >= 0 && index < capacity && "Out of bounds access on fixed array");
        return data[index];
    }

    void insert(T& x, int at) {
        for (int i = at; i < size; ++i) {
            data[i+1] = data[i];
        }
        size+=1;
        assertion(size >= 0 && size <= capacity && "Over allocated.");
        data[at] = x;
    }

    void erase(int at) {
        for (int i = at; i < size; ++i) {
            data[i] = data[i+1];
        }
        size -= 1;
    }

    // erase alternative
    void pop_and_swap(int at) {
        data[at] = data[--size];
        assertion(size >= 0 && size <= capacity && "Impossible?");
    }

    void push(const T x) {
        data[size++] = x;
        assertion(size >= 0 && size <= capacity && "Over allocated.");
    }

    void push_unimportant(const T x) {
        if (size >= capacity) {
            data[size-1] = x;
        } else {
            push(x);  
        }
    }

    T* alloc() {
        T* r = &data[size++];
        assertion(size >= 0 && size <= capacity && "Over allocated.");
        return r;
    }

    // Will not allow crashing, just reuse the last if we have no space.
    T* alloc_unimportant() {
        if (size >= capacity) {
            return &data[size-1];
        }
        return alloc();
    }

    void pop() {
        size--;
        assertion(size >= 0 && size <= capacity && "Impossible?");
    }

    // NOTE: I'm not zeroing out the memory to allow
    // for some specific behaviors that you might want to do.
    void clear() {
        size = 0;
    }

    void zero() {
        zero_memory(data, capacity * sizeof(*data));
        clear();
    }

    T*     data;
    size_t size;
    size_t capacity;
};

#endif
