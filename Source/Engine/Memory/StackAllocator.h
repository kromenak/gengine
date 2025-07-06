//
// Clark Kromenaker
//
// Allocates memory from a provided memory buffer in a stack-like fashion.
//
// Memory can only be allocated from position of last allocation, and deallocations
// can only occur in reverse (LIFO).
//
// This is a fairly simple allocator with good performance.
//
#pragma once
#include <cstddef>
#include <cstdint>

class StackAllocator
{
public:
    StackAllocator(void* memory, size_t size);

    void* Allocate(size_t size, unsigned short alignment = 4);
    void Deallocate(void* memory);

    void Reset();

    size_t GetAllocationCount() const { return mAllocationCount; }
    size_t GetAllocatedSize() const { return mAllocatedSize; }

private:
    struct AllocHeader
    {
        // Mainly for debugging/safety - when a new allocation is made, store previous expected dealloc in header.
        // Then, when deallocating THIS allocation, we can restore it.
        void* prevExpectedDealloc;

        // Offset from memory returned to caller to actual start of block.
        // This is extra space allocated for this header and to satisfy alignment requirements.
        unsigned short allocOffset;
    };

    // The allocator's memory.
    void* mMemory = nullptr;
    size_t mSize = 0;

    // Allocation stats.
    size_t mAllocationCount = 0;
    size_t mAllocatedSize = 0;

    // The "top" of the stack.
    // This represents the position in the memory buffer we will allocate from next.
    void* mTop = nullptr;

    // A stack allocator restriction is LIFO - any memory deallocated must be the last item allocated.
    // For debugging, this is the last pointer returned by alloc.
    // If dealloc gets a memory pointer that IS NOT this, that's likely a critical problem!
    void* mExpectedDealloc = nullptr;
};
