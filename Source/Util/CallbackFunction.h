//
// Clark Kromenaker
//
// A wrapper around a function pointer.
//
#pragma once
#include <cassert>

template<class T>
class CallbackFunction
{
public:
    CallbackFunction(void (*ptr)(T)) : mFuncPtr(ptr) { }
    
    void operator()(T val) const
    {
        assert(mFuncPtr != nullptr);
        mFuncPtr(val);
    }
    
private:
    // Pointer to the function to call.
    void (*mFuncPtr)(T) = nullptr;
};
