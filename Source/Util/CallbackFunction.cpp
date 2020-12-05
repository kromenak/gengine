//
// CallbackFunction.cpp
//
// Clark Kromenaker
//
#include "CallbackFunction.h"

template<class T>
CallbackFunction<T>::CallbackFunction(void (*ptr)(T))
    : mFuncPtr(ptr)
{
    
}

template<class T>
void CallbackFunction<T>::operator()(T val) const
{
    if(mFuncPtr != nullptr) { mFuncPtr(val); }
}
