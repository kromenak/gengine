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

// A "type handler" encapsulates how to perform operations on a particular type.
// It is just a set of functions that know how to copy/delete a type.
struct TypeHandler
{
    void* (*copyFrom)(void* src);
    void (*destroy)(void* p);
    void (*destroyArray)(void* p);
};

// Generates a "type handler" for the given type T.
template<typename T>
TypeHandler* GetTypeHandler()
{
    struct THandler
    {
        static void* CopyFrom(void* p) { return new T(*(T*)p); }
        static void  Destroy(void* p) { delete (T*)p; }
        static void  DestroyArray(void* p) { delete[] (T*)p; }
    };
    static TypeHandler th = { &THandler::CopyFrom, &THandler::Destroy, &THandler::DestroyArray };
    return &th;
}

// Encapsulates data and its type as separate objects, allowing storage of arbitrary types of data in a unforim
// Very simple/barebones - data must be deleted manually.
struct BasicValue
{
    TypeHandler* typeHandler;
    void* data;

    BasicValue() = default;
    template<typename T> BasicValue(T* x) : typeHandler(GetTypeHandler<T>()), data(x) { }

    void* Copy()
    {
        return typeHandler->copyFrom(data);
    }

    void Delete()
    {
        typeHandler->destroy(data);
        data = nullptr;
    }

    void DeleteArray()
    {
        typeHandler->destroyArray(data);
        data = nullptr;
    }

    // Allows us to convert a value back to the original type.
    // But note you MUST know the correct type. The assert will fail otherwise!
    template<typename T>
    T& To() const
    {
        assert(typeHandler == GetTypeHandler<T>());
        return *(T*)data;
    }
};

// Similar to BasicValue, but handles lifecycle of data when copied/destructed.
struct Value
{
    // A piece of memory to hold the value (void*), and TypeHandler that
    // explains how to manipulate that piece of memory, based on whether it's an int, string, float, etc.
    TypeHandler* typeHandler;
    void* data;

    template<typename T> Value(const T& x) : typeHandler(GetTypeHandler<T>()), data(new T(x)) { }
    Value(const Value& other) : typeHandler(other.typeHandler), data(typeHandler->copyFrom(other.data)) { }
    ~Value() { typeHandler->destroy(data); }

    // Assignment between Value objects.
    Value& operator=(const Value& other)
    {
        if(this != &other)
        {
            typeHandler->destroy(data);
            typeHandler = other.typeHandler;
            data = typeHandler->copyFrom(other.data);
        }
        return *this;
    }

    // Allows us to assign any type for the value to hold.
    template<typename T>
    Value& operator=(const T& other)
    {
        typeHandler->destroy(data);
        typeHandler = GetTypeHandler<T>();
        data = new T(other);
        return *this;
    }

    // Allows us to convert a value back to the original type.
    // But note you MUST know the correct type. The assert will fail otherwise!
    template<typename T>
    T& To() const
    {
        assert(typeHandler == GetTypeHandler<T>());
        return *(T*)data;
    }
};
