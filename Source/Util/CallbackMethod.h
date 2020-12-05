//
// CallbackMethod.cpp
//
// Clark Kromenaker
//
// Like a callback function, but includes an object reference.
// And so, we can then call the function on that object.
//
#pragma once
#include "CallbackFunction.h"

template<class T, class U>
class CallbackMethod : public CallbackFunction<U>
{
public:
    CallbackMethod(T* objectInstance, void (T::*ptr)(U));
    
    void operator()(U val) const override;
    
private:
    T* mObjectInstance = nullptr;
    void (T::*mMethodPtr)(U) = nullptr;
};
