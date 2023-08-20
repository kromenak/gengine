//
// Clark Kromenaker
//
// A queue container (first in, first out). New elements go on the back, elements are removed from the front.
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
class Queue
{
public:
    // Iterator for range-based for support. Invalidated on add/remove.
    // Using an iterator for other purposes is probably not worthwhile, imo.
    class Iterator
    {
    public:
        Iterator(uint32_t index, uint32_t remaining, T* data) : index(index), remaining(remaining), data(data) { }
        void operator++() { ++index; index %= TCapacity; --remaining; }
        bool operator==(const Iterator& other) const { return index == other.index && remaining == other.remaining; }
        bool operator!=(const Iterator& other) const { return !(*this == other); }
        const T& operator*() const { return data[index]; }

    private:
        // Track index into data array.
        // To differentiate "empty" and "full", we need to track remaining elements in list.
        uint32_t index;
        uint32_t remaining;
        T* data;
    };
    Iterator begin() { return Iterator(mHead, mSize, reinterpret_cast<T*>(mData)); }
    Iterator end() { return Iterator(mTail, 0, reinterpret_cast<T*>(mData)); }
    
    ~Queue()
    {
        Clear();
    }

    void Push(const T& t)
    {
        assert(mSize < TCapacity);

        // Put the item at the tail.
        reinterpret_cast<T*>(mData)[mTail] = t;

        // Move the tail forward, with wraparound.
        ++mTail;
        mTail %= TCapacity;

        // Increase size.
        ++mSize;
    }

    template<typename... Args>
    void Emplace(Args&&... args)
    {
        assert(mSize < TCapacity);

        // Get pointer to alloc location.
        uint8_t* allocAt = &mData[mTail * sizeof(T)];

        // Allocate a new T at that memory location.
        new(allocAt) T(std::forward<Args>(args)...);

        // Move the tail forward, with wraparound.
        ++mTail;
        mTail %= TCapacity;

        // Increase size.
        ++mSize;
    }

    void Pop()
    {
        assert(mSize > 0);

        // Destruct the popped element.
        reinterpret_cast<T*>(mData)[mHead].~T();

        // Move the head forward, with wraparound.
        ++mHead;
        mHead %= TCapacity;

        // Reduce size.
        --mSize;
    }

    void Clear()
    {
        // Destruct all elements in the queue.
        for(auto& element : *this)
        {
            element.~T();
        }
        mSize = mHead = mTail = 0;
    }

    T& Front()
    {
        assert(mSize > 0);
        return reinterpret_cast<T*>(mData)[mHead];
    }

    T& Back()
    {
        assert(mSize > 0);
        if(mTail == 0)
        {
            return reinterpret_cast<T*>(mData)[TCapacity - 1];
        }
        else
        {
            return reinterpret_cast<T*>(mData)[mTail - 1];
        }
    }

    T& operator[](uint32_t i)
    {
        assert(i < mSize);
        uint32_t index = (mHead + i) % TCapacity;
        return reinterpret_cast<T*>(mData)[index];
    }

    const T& operator[](uint32_t i) const
    {
        assert(i < mSize);
        uint32_t index = (mHead + i) % TCapacity;
        return reinterpret_cast<T*>(mData)[index];
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

    // Number of elements currently in queue.
    uint32_t mSize = 0;

    // The queue is implemented as a circular array. This enables implementing a queue using a contiguous array (rather than a linked list).
    // Items are pushed to the back of the array (moving the tail) and read from the front (moving the head), with wraparound.
    uint32_t mHead = 0;
    uint32_t mTail = 0;
};