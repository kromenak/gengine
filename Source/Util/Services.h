//
// Clark Kromenaker
//
// A locator for globally available services in the game.
//
// Some services have static accessors, but there are also general purpose
// "Get" and "Set" function that should allow *any* instance to be globally available.
//
// This is similar to a Singleton, but is more loosely coupled. A benefit
// of this would be the use of Interfaces to swap services.
//
#pragma once
#include <unordered_map>

#include "TypeId.h"

class Services
{
public:
    template<class T> static void Set(T* instance)
    {
        // Just create a mapping from the type to the instance!
        sTypeToInstancePointer[T::sTypeInfo.GetTypeId()] = instance;
    }

    template<class T> static T* Get()
    {
        // Attempt to retrieve and return the instance from the type.
        auto it = sTypeToInstancePointer.find(T::sTypeInfo.GetTypeId());
        if(it != sTypeToInstancePointer.end())
        {
            return static_cast<T*>(it->second);
        }
        return nullptr;
    }

private:
    // General-purpose mapping from class Type to class instance.
    // Use "Set" to add an entry and "Get" to retrieve an entry.
    static std::unordered_map<TypeId, void*> sTypeToInstancePointer;
};