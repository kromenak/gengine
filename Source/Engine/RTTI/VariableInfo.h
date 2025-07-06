//
// Clark Kromenaker
//
// Stores info about individual member variables within a Type.
// For example: the type and name of the variable, its offset within the Type.
//
#pragma once
#include <cassert>
#include <vector>

#include "PtrMath.h"

class GTypeInfo;

enum class VariableType
{
    Int,
    Float,
    Bool,
    String,

    Vector2,
    Vector3,
    Quaternion,

    Asset
};

//TODO: Support for Arrays (std::vector).
//TODO: Expose variable in tool as read-only.
//TODO: Version that supports getter/setter usage.
class VariableInfo
{
public:
    VariableInfo(VariableType type, const char* name, size_t offset) :
        mType(type),
        mName(name),
        mOffset(offset)
    {

    }

    VariableType GetType() const { return mType; }
    const char* GetName() const { return mName; }

    template<typename T> T* GetPtr(void* instance) { return reinterpret_cast<T*>(PtrMath::Add(instance, mOffset)); }
    template<typename T> T& GetRef(void* instance) { return *GetPtr<T>(instance); }

private:
    // The type of the variable.
    VariableType mType = VariableType::Int;

    // The name of the variable.
    const char* mName = nullptr;

    // This variable's offset from the base address of an object instance.
    size_t mOffset = 0;
};