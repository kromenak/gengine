//
// Clark Kromenaker
//
// A TypeId uniquely identifies a Type that is registered in the Type Database.
// The TypeId for a Type should stay consistent between recompiles, runs of the game, etc.
//
#pragma once
#include <cstdint>

// A type ID is just an integer.
typedef uint32_t TypeId;

// Type ID 0 is reserved for an invalid/unset type.
// It is also used to indicate that we should auto-generate the type ID.
#define INVALID_TYPE_ID 0
#define GENERATE_TYPE_ID 0

// The placeholder base class (see below) uses a special Type ID.
#define NO_BASE_CLASS_TYPE_ID UINT32_MAX