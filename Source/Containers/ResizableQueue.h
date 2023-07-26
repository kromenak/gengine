//
// Clark Kromenaker
//
// A queue container (first in, first out). New elements go on the back, elements are removed from the front.
// Uses a fixed-size contiguous array of elements internally.
// 
// Characteristics:
// - Dynamic size: size can increase if too many elements are added.
// - Contiguous: elements are contiguous in memory.
// - Flexible access: elements can be accessed in any order.
//
#pragma once
#include <cassert>
#include <cstring>
#include <cstdlib>

#include "Atomics.h"

template<typename T>
class ResizableQueue
{
public:
    // Iterator for range-based for support. Invalidated on add/remove.
    // Using an iterator for other purposes is probably not worthwhile, imo.
    class Iterator
    {
    public:
        Iterator(uint32 index, uint32 remaining, T* data, uint32 dataSize) : index(index), remaining(remaining), data(data), dataSize(dataSize) { }
        void operator++() { ++index; index %= dataSize; --remaining; }
        bool operator==(const Iterator& other) const { return index == other.index && remaining == other.remaining; }
        bool operator!=(const Iterator& other) const { return !(*this == other); }
        const T& operator*() const { return data[index]; }

    private:
        // Track index into data array.
        // To differentiate "empty" and "full", we need to track remaining elements in list.
        uint32 index;
        uint32 remaining;
        T* data;
        uint32 dataSize;
    };
    Iterator begin() { return Iterator(mHead, mSize, reinterpret_cast<T*>(mData), mCapacity); }
    Iterator end() { return Iterator(mTail, 0, reinterpret_cast<T*>(mData), mCapacity); }

    ResizableQueue(uint32 initialCapacity) :
        mCapacity(initialCapacity)
    {
        mData = new uint8[initialCapacity * sizeof(T)];
    }

    ~ResizableQueue()
    {
        Clear();
        delete[] mData;
    }

    void Push(const T& t)
    {
        // Increase capacity if out of space.
        if(mSize >= mCapacity)
        {
            IncreaseCapacity();
        }

        // Put the item at the tail.
        reinterpret_cast<T*>(mData)[mTail] = t;

        // Move the tail forward, with wraparound.
        ++mTail;
        mTail %= mCapacity;

        // Increase size.
        ++mSize;
    }

    template<typename... Args>
    void Emplace(Args&&... args)
    {
        // Increase capacity if out of space.
        if(mSize >= mCapacity)
        {
            IncreaseCapacity();
        }

        // Get pointer to alloc location.
        uint8* allocAt = &mData[mTail * sizeof(T)];

        // Allocate a new T at that memory location.
        new(allocAt) T(std::forward<Args>(args)...);

        // Move the tail forward, with wraparound.
        ++mTail;
        mTail %= mCapacity;

        // Increase size.
        ++mSize;
    }

    void Pop()
    {
        // Make sure we don't pop when empty.
        assert(mSize > 0);

        // Destruct the popped element.
        reinterpret_cast<T*>(mData)[mHead].~T();

        // Move the head forward, with wraparound.
        ++mHead;
        mHead %= mCapacity;

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
            return reinterpret_cast<T*>(mData)[mCapacity - 1];
        }
        else
        {
            return reinterpret_cast<T*>(mData)[mTail - 1];
        }
    }

    T& operator[](uint32 i)
    {
        assert(i < mSize);
        uint32 index = (mHead + i) % mCapacity;
        return reinterpret_cast<T*>(mData)[index];
    }

    const T& operator[](uint32 i) const
    {
        assert(i < mSize);
        uint32 index = (mHead + i) % mCapacity;
        return reinterpret_cast<T*>(mData)[index];
    }

    uint32 Capacity() const
    {
        return mCapacity;
    }

    bool Empty() const
    {
        return mSize == 0;
    }

    uint32 Size() const
    {
        return mSize;
    }

private:
    // The array that stores the items.
    uint8* mData = nullptr;

    // The current capacity of the queue.
    uint32 mCapacity = 0;

    // The number of items currently in the queue.
    uint32 mSize = 0;

    // The queue is implemented as a circular array. This enables implementing a queue using a contiguous array (rather than a linked list).
    // Items are pushed to the back of the array (moving the tail) and read from the front (moving the head), with wraparound.
    uint32 mHead = 0;
    uint32 mTail = 0;

    void IncreaseCapacity()
    {
        // Let's just double capacity each time.
        uint32 oldCapacity = mCapacity;
        mCapacity *= 2;

        // Create a new array for elements.
        uint8* newData = new uint8[mCapacity * sizeof(T)];

        // Copy contents from old array to new array.
        memcpy(newData, mData, oldCapacity * sizeof(T));

        // If tail is less than head, that means the range from head to tail "wraps around".
        // But since we now have a much bigger array (twice the size), move looped-around elements to the end.
        if(mSize > 0 && mTail <= mHead)
        {
            uint32 oldIndex = 0;
            while(oldIndex < mTail)
            {
                reinterpret_cast<T*>(newData)[oldCapacity + oldIndex] = reinterpret_cast<T*>(mData)[oldIndex];
                ++oldIndex;
            }
            mTail = oldCapacity + oldIndex;
        }

        // Delete old array and keep pointer to new one.
        delete[] mData;
        mData = newData;
    }
};