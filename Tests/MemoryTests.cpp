//
// Clark Kromenaker
//
// Tests for memory-related functions.
//
#include "catch.hh"

#include <vector>

#include "PtrMath.h"
#include "LinearAllocator.h"
#include "FreestyleAllocator.h"

TEST_CASE("Pointer Add/Subtract/Diff are correct")
{
    // Make sure adding results in the expected value.
    char array[100];
    array[99] = 42;
    char* charPtr1 = (char*)PtrMath::Add(array, 99);
    REQUIRE(*charPtr1 == array[99]);
    REQUIRE(PtrMath::Diff(charPtr1, array) == 99);
    
    // Same for subtraction.
    array[50] = 97;
    char* charPtr2 = (char*)PtrMath::Subtract(charPtr1, 49);
    REQUIRE(*charPtr2 == array[50]);
    REQUIRE(PtrMath::Diff(charPtr1, charPtr2) == 49);
    REQUIRE(PtrMath::Diff(charPtr2, charPtr1) == -49);
    
    // Adding a negative (subtracts).
    array[57] = 124;
    charPtr1 = (char*)PtrMath::Add(&array[67], -10);
    REQUIRE(*charPtr1 == array[57]);
    
    // Subtracting a negative (adds).
    array[71] = 103;
    charPtr1 = (char*)PtrMath::Subtract(&array[65], -6);
    REQUIRE(*charPtr1 == array[71]);
    
    // Check that this works OK with pointers for types larger than 1 byte.
    // Here, each int is 4 bytes, but remember Add/Subtract are byte level only!
    int intArray[25];
    intArray[5] = 8675309;
    int* intPtr = (int*)PtrMath::Add(intArray, 20);
    REQUIRE(*intPtr == intArray[5]);
    REQUIRE(PtrMath::Diff(intPtr, intArray) == 20);
    REQUIRE(PtrMath::Diff(intArray, intPtr) == -20);
    
    // Int subtraction.
    intArray[15] = 409;
    intPtr = (int*)PtrMath::Subtract(&intArray[18], 12);
    REQUIRE(*intPtr == intArray[15]);
    REQUIRE(PtrMath::Diff(&intArray[18], &intArray[15]) == 12);
}

TEST_CASE("Memory alignment math is correct")
{
    // Trivial case: already aligned.
    void* testAddr = (void*)4;
    void* expectedAddr = testAddr;
    void* calcAddr = PtrMath::Align(testAddr, 4);
    REQUIRE(calcAddr == expectedAddr);
    
    // Misaligned a bit.
    testAddr = (void*)5;
    expectedAddr = (void*)8;
    calcAddr = PtrMath::Align(testAddr, 4);
    REQUIRE(calcAddr == expectedAddr);
    
    // A larger alignment requirement.
    testAddr = (void*)45;
    expectedAddr = (void*)128;
    calcAddr = PtrMath::Align(testAddr, 128);
    REQUIRE(calcAddr == expectedAddr);
    
    // Let's try an address that's more likely in a real scenario.
    // This is an actual address assigned to an array element in a test program.
    // Try a few common alignment operations.
    testAddr = (void*)140732920751107;
    
    // Align by 2.
    expectedAddr = (void*)140732920751108;
    calcAddr = PtrMath::Align(testAddr, 2);
    REQUIRE(calcAddr == expectedAddr);
    
    // Align by 4.
    expectedAddr = (void*)140732920751108;
    calcAddr = PtrMath::Align(testAddr, 4);
    REQUIRE(calcAddr == expectedAddr);
    
    // Align by 8.
    expectedAddr = (void*)140732920751112;
    calcAddr = PtrMath::Align(testAddr, 8);
    REQUIRE(calcAddr == expectedAddr);
    
    // Align by 16.
    expectedAddr = (void*)140732920751120;
    calcAddr = PtrMath::Align(testAddr, 16);
    REQUIRE(calcAddr == expectedAddr);
    
    // Align by 32.
    expectedAddr = (void*)140732920751136;
    calcAddr = PtrMath::Align(testAddr, 32);
    REQUIRE(calcAddr == expectedAddr);
    
    // Align by 64.
    expectedAddr = (void*)140732920751168;
    calcAddr = PtrMath::Align(testAddr, 64);
    REQUIRE(calcAddr == expectedAddr);
    
    // Align by 128.
    expectedAddr = (void*)140732920751232;
    calcAddr = PtrMath::Align(testAddr, 128);
    REQUIRE(calcAddr == expectedAddr);
}

TEST_CASE("Memory alignment math (with header) is correct")
{
    // So, let's say my address is already aligned...BUT I need some buffer for my oddly sized header.
    // Start at address 4, add 13 for buffer (17), and then align on 4-byte boundary (20).
    int headerSize = 13;
    void* testAddr = (void*)4;
    void* expectedAddr = (void*)20;
    void* calcAddr = PtrMath::AlignWithHeader(testAddr, 4, headerSize);
    REQUIRE(calcAddr == expectedAddr);
    REQUIRE(PtrMath::Diff(calcAddr, testAddr) > headerSize);
    
    // Try with slightly more crazy numbers.
    headerSize = 57;
    testAddr = (void*)140732920751107;
    expectedAddr = (void*)140732920751360;
    calcAddr = PtrMath::AlignWithHeader(testAddr, 256, headerSize);
    REQUIRE(calcAddr == expectedAddr);
    REQUIRE(PtrMath::Diff(calcAddr, testAddr) > headerSize);
}

TEST_CASE("Linear allocator works correctly")
{
    // Allocate some memory.
    // We know this memory is 4-byte aligned.
    unsigned char memory[1024];
    
    // Create allocator.
    LinearAllocator linearAllocator(memory, 1024);
    REQUIRE(linearAllocator.GetAllocationCount() == 0);
    REQUIRE(linearAllocator.GetAllocatedSize() == 0);
    
    // Make some allocations with no alignment requirement.
    int allocSizes[5] { 5, 4, 2, 1, 3 };
    for(int i = 0; i < 5; ++i)
    {
        linearAllocator.Allocate(allocSizes[i], 1);
    }
    REQUIRE(linearAllocator.GetAllocationCount() == 5);
    REQUIRE(linearAllocator.GetAllocatedSize() == 15);
    
    // Make some allocations with default 4-byte alignment requirement.
    // Alloc 5 bytes: aligns at 16, sets top to 21.
    linearAllocator.Allocate(5);
    REQUIRE(linearAllocator.GetAllocationCount() == 6);
    REQUIRE(linearAllocator.GetAllocatedSize() == 21);
    
    // Alloc 6 bytes: aligns at 24, sets top to 30.
    linearAllocator.Allocate(6);
    REQUIRE(linearAllocator.GetAllocationCount() == 7);
    REQUIRE(linearAllocator.GetAllocatedSize() == 30);
    
    // Alloc 900 bytes: aligns at 32, sets top to 932.
    void* alloc = linearAllocator.Allocate(900);
    REQUIRE(linearAllocator.GetAllocationCount() == 8);
    REQUIRE(linearAllocator.GetAllocatedSize() == 932);
    
    // Try to dealloc previous alloc - should have no effect!
    linearAllocator.Deallocate(alloc);
    REQUIRE(linearAllocator.GetAllocationCount() == 8);
    REQUIRE(linearAllocator.GetAllocatedSize() == 932);
    
    // Try to allocate beyond the capacity of the allocator.
    // Should return nullptr.
    alloc = linearAllocator.Allocate(512);
    REQUIRE(alloc == nullptr);
    REQUIRE(linearAllocator.GetAllocationCount() == 8);
    REQUIRE(linearAllocator.GetAllocatedSize() == 932);
    
    // This allocation should fail.
    // Even though there IS exactly 91 bytes left, with alignment requirement, it can't be done.
    linearAllocator.Allocate(1); // Sets top to 933.
    alloc = linearAllocator.Allocate(91); // Need 3 bytes to align this, but not enough.
    REQUIRE(alloc == nullptr);
    
    // This should work, however - leaves enough room for alignment.
    alloc = linearAllocator.Allocate(88);
    REQUIRE(alloc != nullptr);
    REQUIRE(linearAllocator.GetAllocationCount() == 10);
    REQUIRE(linearAllocator.GetAllocatedSize() == 1024);
    
    // Aaaand we can reset.
    linearAllocator.Reset();
    REQUIRE(linearAllocator.GetAllocationCount() == 0);
    REQUIRE(linearAllocator.GetAllocatedSize() == 0);
    
    // And allocate more.
    alloc = linearAllocator.Allocate(900);
    REQUIRE(alloc != nullptr);
    REQUIRE(linearAllocator.GetAllocationCount() == 1);
    REQUIRE(linearAllocator.GetAllocatedSize() == 900);
}

TEST_CASE("Freestyle allocator basic alloc/dealloc works correctly")
{
    // The checks in this test assume a 64-bit machine...so ignore on Win32 for now.
    #if !defined(_WIN32)
    // Allocate some memory.
    // We know this memory is 4-byte aligned.
    unsigned char memory[1024];
    
    // Create allocator.
    FreestyleAllocator allocator(memory, 1024);
    REQUIRE(allocator.GetAllocationCount() == 0);
    REQUIRE(allocator.GetAllocatedSize() == 0);
    REQUIRE(allocator.GetFreeBlockSize(0) == 1024);
    
    // Allocate 4 blocks of size 128.
    // Because header is 16 bytes, this actually allocates 144 bytes each time.
    std::vector<void*> allocs128;
    for(int i = 0; i < 4; ++i)
    {
        allocs128.push_back(allocator.Allocate(128));
    }
    REQUIRE(allocator.GetAllocationCount() == 4);
    REQUIRE(allocator.GetAllocatedSize() == 576);
    REQUIRE(allocator.GetFreeBlockSize(0) == 448);
    
    // Try to deallocate a block in the middle.
    // This creates a hole of size 144 as free list head, hole of 448 at end.
    allocator.Deallocate(allocs128[2]);
    REQUIRE(allocator.GetAllocationCount() == 3);
    REQUIRE(allocator.GetAllocatedSize() == 432);
    REQUIRE(allocator.GetFreeBlockSize(0) == 144);
    REQUIRE(allocator.GetFreeBlockSize(1) == 448);
    
    // Deallocate first allocated block.
    // Should become new head free block, with next hole 144 and next hole 448.
    allocator.Deallocate(allocs128[0]);
    REQUIRE(allocator.GetAllocationCount() == 2);
    REQUIRE(allocator.GetAllocatedSize() == 288);
    REQUIRE(allocator.GetFreeBlockSize(0) == 144);
    REQUIRE(allocator.GetFreeBlockSize(1) == 144);
    REQUIRE(allocator.GetFreeBlockSize(2) == 448);
    
    // Try to deallocate last allocated block.
    // This one's fun b/c it bridges the last two free blocks. So, it merges with prev AND next!
    allocator.Deallocate(allocs128[3]);
    REQUIRE(allocator.GetAllocationCount() == 1);
    REQUIRE(allocator.GetAllocatedSize() == 144);
    REQUIRE(allocator.GetFreeBlockSize(0) == 144);
    REQUIRE(allocator.GetFreeBlockSize(1) == 736);
    
    // Deallocate second allocated block.
    // Again, this bridges a gap between two free blocks, merging all into one big block.
    allocator.Deallocate(allocs128[1]);
    REQUIRE(allocator.GetAllocationCount() == 0);
    REQUIRE(allocator.GetAllocatedSize() == 0);
    REQUIRE(allocator.GetFreeBlockSize(0) == 1024);
    REQUIRE(allocator.GetFreeBlockSize(1) == 0); // there is no second free block
    #endif
}
