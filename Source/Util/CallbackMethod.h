//
// Clark Kromenaker
//
// Like a callback function, but includes an object reference.
// And so, we can then call the function on that object.
//
#pragma once
#include <cassert>

template<class T, class U>
class CallbackMethod
{
public:
    CallbackMethod(T* objectInstance, void (T::* ptr)(U)) :
        mObjectInstance(objectInstance),
        mMethodPtr(ptr)
    {
    }
    
    void operator()(U val) const
    {
        assert(mObjectInstance != nullptr && mMethodPtr != nullptr);
        (mObjectInstance->*mMethodPtr)(val);
    }
    
private:
    T* mObjectInstance = nullptr;
    void (T::*mMethodPtr)(U) = nullptr;
};
