//
// CallbackMethod.cpp
//
// Clark Kromenaker
//
#include "CallbackMethod.h"

template<class T, class U>
CallbackMethod<T, U>::CallbackMethod(T* objectInstance, void (T::*ptr)(U)) :
    CallbackFunction<U>(nullptr),
    mObjectInstance(objectInstance),
    mMethodPtr(ptr)
{
    
}

template<class T, class U>
void CallbackMethod<T, U>::operator()(U val) const
{
    if(mObjectInstance != nullptr)
    {
        (mObjectInstance->*mMethodPtr)(val);
    }
}
