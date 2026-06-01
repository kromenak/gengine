#if 0
#include <cstdlib>
#include <iostream>

// Replacements for default C++ new/new[] and delete/delete[].
// Overriding the default functions gives us a way to "meter" when memory is allocated or deleted.
void* operator new(size_t size)
{
    return std::malloc(size);
}

void operator delete(void* mem) noexcept
{
    std::free(mem);
}

void operator delete(void* mem, size_t size) noexcept
{
    std::free(mem);
}

void* operator new[](size_t size)
{
    return std::malloc(size);
}

void operator delete[](void* mem) noexcept
{
    std::free(mem);
}

void operator delete[](void* mem, size_t size) noexcept
{
    std::free(mem);
}
#endif