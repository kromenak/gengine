#include "StackAllocator.h"

#include <cassert>

#include "PtrMath.h"

StackAllocator::StackAllocator(void* memory, size_t size) :
    mMemory(memory),
    mSize(size)
{
    Reset();
}

void* StackAllocator::Allocate(size_t size, unsigned short alignment)
{
    // Align memory and calculate what buffer that added.
    void* alignedMemory = PtrMath::AlignWithHeader(mTop, alignment, sizeof(AllocHeader));
    int headerSize = PtrMath::Diff(alignedMemory, mTop);
    size += headerSize;

    // If not enough memory left, return nullptr!
    if(mAllocatedSize + size > mSize) { return nullptr; }

    // Create header in space right before aligned memory.
    AllocHeader* header = (AllocHeader*)PtrMath::Subtract(alignedMemory, sizeof(AllocHeader));
    header->prevExpectedDealloc = mExpectedDealloc;
    header->allocOffset = headerSize;

    // Perform allocation.
    mTop = PtrMath::Add(mTop, size);
    mAllocatedSize += size;
    ++mAllocationCount;

    // Return memory.
    mExpectedDealloc = alignedMemory;
    return alignedMemory;
}

void StackAllocator::Deallocate(void* memory)
{
    // Stack is LIFO, so we can only deallocate the last allocation.
    // Trying to deallocate anything else is a developer error.
    assert(memory == mExpectedDealloc);

    // Grab header, stored in memory right before the memory to deallocate.
    AllocHeader* header = (AllocHeader*)PtrMath::Subtract(memory, sizeof(AllocHeader));

    // Determine how much memory to deallocate.
    // Diff between top and provided pointer, sure - but also take into account the header size.
    int deallocateSize = PtrMath::Diff(mTop, memory) + header->allocOffset;

    // Perform deallocation.
    mTop = PtrMath::Subtract(mTop, deallocateSize);
    mAllocatedSize -= deallocateSize;
    --mAllocationCount;

    // Revert expected dealloc to value in this header.
    // Which allows the next item to be popped off the stack.
    mExpectedDealloc = header->prevExpectedDealloc;
}

void StackAllocator::Reset()
{
    // Reset top to beginning of memory block.
    mTop = mMemory;
    mExpectedDealloc = nullptr;

    // Reset stats.
    mAllocatedSize = 0;
    mAllocationCount = 0;
}
