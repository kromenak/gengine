//
// Clark Kromenaker
//
// A stack container (last in, first out). New elements go on the back, elements are removed from the back.
// Uses a fixed-size contiguous array of elements internally.
// 
// Characteristics:
// - Fixed size: max container size must be known at compile time.
// - Contiguous: elements are contiguous in memory.
// - Flexible access: elements can be accessed in any order.
//
#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>

template<typename T, uint32_t TCapacity>
class Stack
{
public:
    // Iterator for range-based for support. Invalidated on add/remove.
    // Using an iterator for other purposes is probably not worthwhile, imo.
    class Iterator
    {
    public:
        Iterator(uint32_t index, T* data) : index(index), data(data) { }
        void operator++() { --index; }
        bool operator==(const Iterator& other) const { return index == other.index; }
        bool operator!=(const Iterator& other) const { return index != other.index;  }
        const T& operator*() const { return data[index - 1]; }

    private:
        // The "index" here is +1 the actual index. This is so "end" can be represented with index 0.
        uint32_t index;
        T* data;
    };
    Iterator begin() { return Iterator(mSize, reinterpret_cast<T*>(mData)); }
    Iterator end() { return Iterator(0, reinterpret_cast<T*>(mData)); }

    ~Stack()
    {
        Clear();
    }

    void Push(const T& t)
    {
        assert(mSize < TCapacity);

        // Put the element at the tail.
        reinterpret_cast<T*>(mData)[mSize] = t;

        // Increase size.
        ++mSize;
    }

    template<typename... Args>
    void Emplace(Args&&... args)
    {
        assert(mSize < TCapacity);

        // Get pointer to alloc location.
        uint8_t* allocAt = &mData[mSize * sizeof(T)];

        // Allocate a new T at that memory location.
        new(allocAt) T(std::forward<Args>(args)...);

        // Increase size.
        ++mSize;
    }

    void Pop()
    {
        // Make sure we don't pop when empty.
        assert(mSize > 0);

        // Reduce size.
        --mSize;

        // Destruct the element we are popping.
        reinterpret_cast<T*>(mData)[mSize].~T();
    }

    void Clear()
    {
        // We must destruct all elements in the queue.
        for(auto& element : *this)
        {
            element.~T();
        }
        mSize = 0;
    }

    T& Front()
    {
        assert(mSize > 0);
        return reinterpret_cast<T*>(mData)[0];
    }

    T& Back()
    {
        assert(mSize > 0);
        return reinterpret_cast<T*>(mData)[mSize - 1];
    }

    T& operator[](uint32_t i)
    {
        assert(i < mSize);
        return reinterpret_cast<T*>(mData)[i];
    }

    const T& operator[](uint32_t i) const
    {
        assert(i < mSize);
        return reinterpret_cast<T*>(mData)[i];
    }

    uint32_t Capacity() const
    {
        return TCapacity;
    }

    bool Empty() const
    {
        return mSize == 0;
    }

    uint32_t Size() const
    {
        return mSize;
    }

private:
    // Memory to store elements.
    alignas(alignof(T)) uint8_t mData[TCapacity * sizeof(T)] = { 0 };

    // Number of elements currently in stack.
    uint32_t mSize = 0;
};