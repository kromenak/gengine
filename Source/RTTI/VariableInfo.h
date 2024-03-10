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

class TypeInfo;

enum class VariableType
{
    Int,
    Float,
    Bool,
    String
};

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

    template<typename T>
    T& GetRef(void* instance)
    {
        return *static_cast<T*>(PtrMath::Add(instance, mOffset));
    }
    template<>
    int& GetRef<int>(void* instance)
    {
        assert(mType == VariableType::Int);
        return *static_cast<int*>(PtrMath::Add(instance, mOffset));
    }

private:
    // The type of the member.
    VariableType mType = VariableType::Int;

    // The name of the member.
    const char* mName = nullptr;

    // This member's offset from the base address of an object instance.
    size_t mOffset = 0;
};