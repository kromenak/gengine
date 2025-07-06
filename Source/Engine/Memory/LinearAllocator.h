//
// Clark Kromenaker
//
// Allocates memory from a provided memory buffer in a linear fashion.
//
// Memory can only be allocated from position of last allocation (linearly)
// and past allocations can't be deallocated individually (must deallocate the whole buffer).
//
// This is a very simple allocator with good performance. But it is rather restrictive.
//
#pragma once
#include <cstddef>

class LinearAllocator
{
public:
    LinearAllocator(void* memory, size_t size);

    void* Allocate(size_t size, unsigned short alignment = 4);
    void Deallocate(void* memory);

    void Reset();

    size_t GetAllocationCount() const { return mAllocationCount; }
    size_t GetAllocatedSize() const { return mAllocatedSize; }

private:
    // The allocator's memory block.
    void* mMemory = nullptr;
    size_t mSize = 0;

    // Allocation stats.
    size_t mAllocationCount = 0;
    size_t mAllocatedSize = 0;

    // This represents the position in the memory buffer we will allocate from next.
    void* mTop = nullptr;
};
