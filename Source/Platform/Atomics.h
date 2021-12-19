//
// Clark Kromenaker
//
// Aliases for "atomic" data types, like char, short, int, etc.
// Using these ensures that variables are the desired size and signedness.
// 
#pragma once
#include <cstdint>

// Integer types with specific sizes.
typedef uint8_t uint8;
typedef int8_t int8;

typedef uint16_t uint16;
typedef int16_t int16;

typedef uint32_t uint32;
typedef int32_t int32;

typedef uint64_t uint64;
typedef int64_t int64;
