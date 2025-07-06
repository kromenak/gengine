#include "LinearAllocator.h"

#include "PtrMath.h"

LinearAllocator::LinearAllocator(void* memory, size_t size) :
    mMemory(memory),
    mSize(size)
{
    Reset();
}

void* LinearAllocator::Allocate(size_t size, unsigned short alignment)
{
    // Align memory and calculate what buffer that added.
    void* alignedMemory = PtrMath::Align(mTop, alignment);
    int headerSize = PtrMath::Diff(alignedMemory, mTop);
    size += headerSize;

    // If not enough memory left, return nullptr!
    if(mAllocatedSize + size > mSize) { return nullptr; }

    // Perform allocation.
    mTop = PtrMath::Add(mTop, size);
    mAllocatedSize += size;
    ++mAllocationCount;

    // Return memory.
    return alignedMemory;
}

void LinearAllocator::Deallocate(void* memory)
{
    // Does nothing!
}

void LinearAllocator::Reset()
{
    // Reset top to beginning of memory block.
    mTop = mMemory;

    // Reset memory stats.
    mAllocationCount = 0;
    mAllocatedSize = 0;
}
