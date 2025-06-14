//
// Clark Kromenaker
//
// An allocator that can accommodate allocations and deallocations of any size and in any order.
//
// This is the most flexible allocator - you can allocate and deallocate pretty much however you want!
// But the tradeoff is that it's more complex and maybe slower.
//
// Also, this is the only allocator that can suffer from fragmentation, which is a whole thing.
//
#pragma once
#include <cstddef>
#include <cstdint>

class FreestyleAllocator
{
public:
    FreestyleAllocator(void* memory, size_t size);

    void* Allocate(size_t size, unsigned short alignment = 4);
    void Deallocate(void* memory);

    void Reset();

    size_t GetAllocationCount() const { return mAllocationCount; }
    size_t GetAllocatedSize() const { return mAllocatedSize; }

    // Helper for automated tests.
    // Don't know if it'd ever come in handy in production!
    size_t GetFreeBlockSize(int i);

private:
    struct AllocHeader
    {
        // The full size of this allocation (including offset amount).
        size_t allocSize;

        // Offset from memory returned to caller to actual start of block.
        // This is extra space allocated for this header and to satisfy alignment requirements.
        unsigned short allocOffset;
    };

    struct FreeBlock
    {
        // The size of this free block (including space taken up by this struct itself).
        size_t size;

        // Location of next free block in linked list.
        FreeBlock* next;
    };

    // The allocator's memory.
    void* mMemory = nullptr;
    size_t mSize = 0;

    // Allocation stats.
    size_t mAllocationCount = 0;
    size_t mAllocatedSize = 0;

    // First free block available. Forms a linked list of free blocks.
    // Helpful note: b/c always linked in order, a block's address is always greater than the last block.
    FreeBlock* mFreeListHead = nullptr;
};
