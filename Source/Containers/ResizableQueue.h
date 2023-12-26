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
#include <cstdint>
#include <cstdlib>

template<typename T>
class ResizableQueue
{
public:
    // Iterator for range-based for support. Invalidated on add/remove.
    // Using an iterator for other purposes is probably not worthwhile, imo.
    class Iterator
    {
    public:
        Iterator(uint32_t index, uint32_t remaining, T* data, uint32_t dataSize) : index(index), remaining(remaining), data(data), dataSize(dataSize) { }
        void operator++() { ++index; index %= dataSize; --remaining; }
        bool operator==(const Iterator& other) const { return index == other.index && remaining == other.remaining; }
        bool operator!=(const Iterator& other) const { return *this != other; }
        const T& operator*() const { return data[index]; }

    private:
        // Track index into data array.
        // To differentiate "empty" and "full", we need to track remaining elements in list.
        uint32_t index;
        uint32_t remaining;
        T* data;
        uint32_t dataSize;
    };
    Iterator begin() { return Iterator(mHead, mSize, reinterpret_cast<T*>(mData), mCapacity); }
    Iterator end() { return Iterator(mTail, 0, reinterpret_cast<T*>(mData), mCapacity); }

    ResizableQueue(uint32_t initialCapacity) :
        mCapacity(initialCapacity)
    {
        mData = new uint8_t[initialCapacity * sizeof(T)];
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
        uint8_t* allocAt = &mData[mTail * sizeof(T)];

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

    T& operator[](uint32_t i)
    {
        assert(i < mSize);
        uint32_t index = (mHead + i) % mCapacity;
        return reinterpret_cast<T*>(mData)[index];
    }

    const T& operator[](uint32_t i) const
    {
        assert(i < mSize);
        uint32_t index = (mHead + i) % mCapacity;
        return reinterpret_cast<T*>(mData)[index];
    }

    uint32_t Capacity() const
    {
        return mCapacity;
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
    // The array that stores the items.
    uint8_t* mData = nullptr;

    // The current capacity of the queue.
    uint32_t mCapacity = 0;

    // The number of items currently in the queue.
    uint32_t mSize = 0;

    // The queue is implemented as a circular array. This enables implementing a queue using a contiguous array (rather than a linked list).
    // Items are pushed to the back of the array (moving the tail) and read from the front (moving the head), with wraparound.
    uint32_t mHead = 0;
    uint32_t mTail = 0;

    void IncreaseCapacity()
    {
        // Let's just double capacity each time.
        uint32_t oldCapacity = mCapacity;
        mCapacity *= 2;

        // Create a new array for elements.
        uint8_t* newData = new uint8_t[mCapacity * sizeof(T)];

        // Copy contents from old array to new array.
        memcpy(newData, mData, oldCapacity * sizeof(T));

        // If tail is less than head, that means the range from head to tail "wraps around".
        // But since we now have a much bigger array (twice the size), move looped-around elements to the end.
        if(mSize > 0 && mTail <= mHead)
        {
            uint32_t oldIndex = 0;
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