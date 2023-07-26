#include "FreestyleAllocator.h"

#include <cassert>

#include "PtrMath.h"

FreestyleAllocator::FreestyleAllocator(void* memory, size_t size) :
    mMemory(memory),
    mSize(size)
{
    assert(mSize > sizeof(FreeBlock));
    Reset();
}

void* FreestyleAllocator::Allocate(size_t size, unsigned short alignment)
{
    // These will be populated when a big enough block is found.
    void* alignedMemory = nullptr;
    int allocOffset = 0;
    int allocSize = 0;
    
    // We need to find a block that is big enough for this allocation.
    FreeBlock* prevBlock = nullptr;
    FreeBlock* block = mFreeListHead;
    while(block != nullptr)
    {
        // See if this block is big enough after accommodating alignment needs.
        alignedMemory = PtrMath::AlignWithHeader((void*)block, alignment, sizeof(AllocHeader));
        allocOffset = PtrMath::Diff(alignedMemory, (void*)block);
        allocSize = size + allocOffset;
        
        // We need to allocate the desired size, PLUS enough for alignment needs and header.
        // If this block isn't big enough with all that, we can't use it.
        if(allocSize <= block->size)
        {
            // This block IS big enough - let's use it.
            break;
        }
        
        // Ok, this block was no good - try the next one.
        prevBlock = block;
        block = block->next;
    }
    
    // Whoops, looks like we don't have any block to accommodate this allocation.
    if(block == nullptr)
    {
        return nullptr;
    }
    
    // Ok, so we are going to allocate from block.
    // BUT we need to take any excess in the block and convert it to a free block.
    size_t excessSize = block->size - allocSize;
    
    // That is, unless, the excess is too small!
    // In that case, we'll just roll the extra memory into this allocation.
    if(excessSize < sizeof(FreeBlock) || excessSize < sizeof(AllocHeader))
    {
        allocSize += excessSize;
    }
    else
    {
        // Excess size is big enough to be it's own block.
        // So, let's create that and hook it up to the free block linked list.
        FreeBlock* freeBlock = (FreeBlock*)PtrMath::Add((void*)block, allocSize);
        freeBlock->size = excessSize;
        freeBlock->next = block->next;
        if(prevBlock != nullptr)
        {
            prevBlock->next = freeBlock;
        }
        else
        {
            // No prevBlock indicates this is the new linked list head.
            mFreeListHead = freeBlock;
        }
    }
    
    // Create header for this allocation.
    AllocHeader* header = (AllocHeader*)PtrMath::Subtract(alignedMemory, sizeof(AllocHeader));
    header->allocOffset = allocOffset;
    header->allocSize = allocSize;
    
    // Perform allocation.
    mAllocatedSize += allocSize;
    ++mAllocationCount;
    
    // Return memory.
    return alignedMemory;
}

void FreestyleAllocator::Deallocate(void* memory)
{
    // Grab header, stored in memory right before the memory to deallocate.
    AllocHeader* header = (AllocHeader*)PtrMath::Subtract(memory, sizeof(AllocHeader));
    int allocSize = header->allocSize;
    
    // Get pointer to start of block (before header and alignment offset).
    void* blockStart = PtrMath::Subtract(memory, header->allocOffset);
    
    // Create free block here.
    FreeBlock* freeBlock = (FreeBlock*)blockStart;
    freeBlock->size = allocSize;
    
    // Add free block to linked list.
    // Need to find where this block "slots in" in the free list.
    FreeBlock* prevPrevBlock = nullptr;
    FreeBlock* prevBlock = nullptr;
    FreeBlock* nextBlock = mFreeListHead;
    while(nextBlock != nullptr)
    {
        // If free block's address is less than current block, it means
        // free block slots in after prev (if any) and before curr!
        if(PtrMath::LessThan(freeBlock, nextBlock))
        {
            // If no gap between this free block and previous one, merge them into one big free block!
            if(prevBlock != nullptr && PtrMath::Diff(freeBlock, prevBlock) == prevBlock->size)
            {
                // Prev block will "absorb" the block we were freeing.
                // So, pretend prev block is the block we're adding.
                prevBlock->size += freeBlock->size;
                freeBlock = prevBlock;
                prevBlock = prevPrevBlock;
            }
            
            // Also, if no gap between free block and the curr block, also merge into a bigger free block!
            int diff = PtrMath::Diff(nextBlock, freeBlock);
            if(diff == freeBlock->size)
            {
                freeBlock->size += nextBlock->size;
                nextBlock = nextBlock->next;
            }
            
            // Add to linked list by patching prev/next.
            freeBlock->next = nextBlock;
            if(prevBlock != nullptr)
            {
                prevBlock->next = freeBlock;
            }
            else
            {
                // No prevBlock indicates this is the new linked list head.
                mFreeListHead = freeBlock;
            }
            break;
        }
        
        // Free block doesn't fit here - advance to next check.
        prevPrevBlock = prevBlock;
        prevBlock = nextBlock;
        nextBlock = nextBlock->next;
    }
    
    // Perform deallocation.
    mAllocatedSize -= allocSize;
    --mAllocationCount;
}

void FreestyleAllocator::Reset()
{
    // Destroy the free list and just treat all our memory as one giant block.
    mFreeListHead = (FreeBlock*)mMemory;
    mFreeListHead->size = mSize;
    mFreeListHead->next = nullptr;
    
    // Reset stats.
    mAllocationCount = 0;
    mAllocatedSize = 0;
}

size_t FreestyleAllocator::GetFreeBlockSize(int i)
{
    // Iterate free list to ith block.
    // 0 = head, 1 = next, 2 = next next, etc.
    FreeBlock* current = mFreeListHead;
    while(current != nullptr && i > 0)
    {
        current = current->next;
        --i;
    }
    
    // Trying to get ith block, but there are fewer than i blocks.
    // Just default to zero in that case.
    if(current == nullptr) { return 0; }
    
    // Return the size.
    return current->size;
}
