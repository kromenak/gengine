//
// Value.h
//
// Clark Kromenaker
//
// A container that can hold a value of any type. Allows you to treat all
// possible types for a variable as a homogenous "value" object.
//
// Once a value is assigned, the only way to get the value back is to know the type.
// Otherwise, it just points to a piece of memory with an unknown way to interpret it.
//
// Based off of "boost.any" type, and taken from here:
// https://stackoverflow.com/questions/27670222/mapping-functions-with-variable-arguments-and-calling-by-string-c
//
#pragma once
#include <cassert>

// Contains points to functions that know how to copy or destroy a particular type.
struct TypeHandler
{
    void* (*copyFrom)(void *src);
    void (*destroy)(void *p);
};

// A templated function that will generate unique TypeHandlers based on type provided.
template<typename T>
TypeHandler *thandler()
{
    struct THandler
    {
        static void *copyFrom(void *p) { return new T(*(T *)p); }
        static void destroy(void *p) { delete (T *)p; }
    };
    static TypeHandler th = { &THandler::copyFrom, &THandler::destroy };
    return &th;
}

// A struct that contains some arbitrary type stored as a void pointer.
// But the interface knows how to assign to and take from that piece of memory.
struct Value
{
    // A piece of memory to hold the value (void*), and TypeHandler that
    // explains how to manipulate that piece of memory, based on whether it's an int, string, float, etc.
    TypeHandler *th;
    void *p;
    
    Value(const Value& other) : th(other.th), p(th->copyFrom(other.p)) { }
    template<typename T> Value(const T& x) : th(thandler<T>()), p(new T(x)) { }
    ~Value() { th->destroy(p); }
    
    // Assignment between Value objects.
    Value& operator=(const Value& other)
    {
        if (this != &other)
        {
            th->destroy(p);
            th = other.th;
            p = th->copyFrom(other.p);
        }
        return *this;
    }
    
    // Allows us to assign any type for the value to hold.
    template<typename T>
    Value& operator=(const T& other)
    {
        th->destroy(p);
        th = thandler<T>();
        p = new T(other);
        return *this;
    }
    
    // Allows us to convert a value back to the original type.
    // But note you MUST know the correct type. The assert will fail otherwise!
    template<typename T>
    T& to() const
    {
        assert(th == thandler<T>());
        return *(T*)p;
    }
};
