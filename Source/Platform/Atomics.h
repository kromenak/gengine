//
// Atomics.h
//
// Clark Kromenaker
//
// Aliases for "atomic" data types, like char, short, int, etc.
// Using these ensures that variables are the desired size and signedness.
// 
#pragma once
#include <cstdint>

// Integer types with specific sizes.
typedef uint8_t U8;
typedef int8_t I8;

typedef uint16_t U16;
typedef int16_t I16;

typedef uint32_t U32;
typedef int32_t I32;

typedef uint64_t U64;
typedef int64_t I64;

// Floating-point types are more well defined, but for completion's sake...
typedef float F32;
typedef double F64;
