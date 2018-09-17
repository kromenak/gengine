//
// CallbackFunction.cpp
//
// Clark Kromenaker
//
// A wrapper around a function pointer.
//
#pragma once

template<class T>
class CallbackFunction
{
public:
    CallbackFunction(void (*ptr)(T));
    
    virtual void operator()(T val) const;
    
private:
    // Pointer to the function to call.
    void (*mFuncPtr)(T) = nullptr;
};
