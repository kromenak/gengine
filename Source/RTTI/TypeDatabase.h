#pragma once
#include <cassert>
#include <unordered_map>
#include <vector>

#include "TypeId.h"
#include "TypeInfo.h"

class TypeDatabase
{
public:
    static TypeDatabase& Get()
    {
        // A global doesn't work due to order-of-initialization issues.
        // In short, static TypeInfos may try to register with TypeDatabase *before* TypeDatabase is initialized itself!

        // To fix that, use a static local variable to take control of when initialization occurs!
        static TypeDatabase database;
        return database;
    }

    void RegisterType(GTypeInfo* typeInfo)
    {
        // Not going to check for duplicates - it should naturally not happen, right?
        mTypes.push_back(typeInfo);

        // Add to map. In debug, ensure there are no duplicate type IDs.
        assert(mTypeIdToTypeInfo.find(typeInfo->GetTypeId()) == mTypeIdToTypeInfo.end());
        mTypeIdToTypeInfo[typeInfo->GetTypeId()] = typeInfo;
    }

private:
    // All Types register themselves automatically on construction.
    // So, this list is assumed to contain all Types in the game that have TypeInfo!
    std::vector<GTypeInfo*> mTypes;

    // Maps a type ID to its type info.
    std::unordered_map<TypeId, GTypeInfo*> mTypeIdToTypeInfo;
};