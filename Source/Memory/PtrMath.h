//
// Clark Kromenaker
//
// Helpers for performing calculations with pointers.
//
#pragma once
#include <cstddef>
#include <cstdint>

namespace PtrMath
{
    inline void* Add(void* pointer, int amount)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
        address += amount;
        return reinterpret_cast<void*>(address);
    }

    inline void* Subtract(void* pointer, int amount)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
        address -= amount;
        return reinterpret_cast<void*>(address);
    }

    inline ptrdiff_t Diff(void* lhs, void* rhs)
    {
        uintptr_t lhsAddr = reinterpret_cast<uintptr_t>(lhs);
        uintptr_t rhsAddr = reinterpret_cast<uintptr_t>(rhs);
        return static_cast<ptrdiff_t>(lhsAddr - rhsAddr);
    }

    inline bool LessThan(void* lhs, void* rhs)
    {
        uintptr_t lhsAddr = reinterpret_cast<uintptr_t>(lhs);
        uintptr_t rhsAddr = reinterpret_cast<uintptr_t>(rhs);
        return lhsAddr < rhsAddr;
    }

    /**
     * Given a pointer, returns the nearest larger pointer whose address is aligned as desired.
     * "Aligned" means that the address is divisible by the alignment value.
     *
     * Note that alignment must/should be a power of 2!
     */
    inline void* Align(void* pointer, unsigned short alignment)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
        address += (alignment - 1);
        address = address & (~(alignment - 1));
        return reinterpret_cast<void*>(address);
    }

    /**
     * Like Align above, but also allows allocating some extra space to store a header.
     * Headers are required by some types of allocators.
     *
     * Note that alignment must/should be a power of 2!
     */
    inline void* AlignWithHeader(void* pointer, unsigned short alignment, unsigned short headerSize)
    {
        // See if meeting alignment requirement will naturally leave enough room for header.
        void* alignedPointer = Align(pointer, alignment);
        if(Diff(alignedPointer, pointer) >= headerSize) { return alignedPointer; }

        // Normal alignment doesn't leave enough room for the header, so we'll have to manually add space.
        // Pretend that the address passed in was after some number of bytes for a header.
        uintptr_t address = reinterpret_cast<uintptr_t>(pointer) + headerSize;

        // Then, align per usual.
        address += (alignment - 1);
        address = address & (~(alignment - 1));
        return reinterpret_cast<void*>(address);
    }
}