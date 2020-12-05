//
// Type.h
//
// Clark Kromenaker
//
// Defines a "Type" type and macros for adding runtime type info (RTTI) to a
// base class and subclasses.
//
#include <cstddef>
#include <functional>   // for std::hash support
#include <string>

// A class's type will just be a size_t alias right now.
typedef std::size_t Type;

// Converts passed in symbol to a string (MyClass => "MyClass").
#define SYMBOL_TO_STR(x) #x

// Generates a Type for a particular symbol.
// Allows a "Type" to be generated for *any* class, not just those that use TYPE_DECL/DEF macros below.
// Note: std::hash will not necessarily return the same value across runs of a program!
#define GENERATE_TYPE(x) std::hash<std::string>()(SYMBOL_TO_STR(x));

// Add inside of a base class declaration to add type info to that base class.
// First class in class hierarchy that needs type info must specify this class.
#define TYPE_DECL_BASE()                                              \
private:                                                            \
    static const Type type;                                         \
                                                                    \
public:                                                             \
    static Type GetType() { return type; }                          \
    virtual bool IsTypeOf(const Type t) const { return t == type; } \

// Add in base class definition (bottom of header or impl file).
#define TYPE_DEF_BASE(nameOfClass)                                                  \
const Type nameOfClass::type = std::hash<std::string>()(SYMBOL_TO_STR(nameOfClass));  \

// Adds type data to a child class. The base class must have declared TYPE_DECL_BASE.
// Include this macro in the class declaration/header.
#define TYPE_DECL_CHILD()                                   \
private:                                                    \
    static const Type type;                                 \
                                                            \
public:                                                     \
    static Type GetType() { return type; }                  \
    virtual bool IsTypeOf(const Type t) const override;     \

// Include this macro in the class definition (bottom of header or impl file).
#define TYPE_DEF_CHILD(nameOfParentClass, nameOfChildClass)                                         \
const Type nameOfChildClass::type = std::hash<std::string>()(SYMBOL_TO_STR(nameOfChildClass));        \
                                                                                                    \
bool nameOfChildClass::IsTypeOf(const Type t) const {                                               \
    return t == nameOfChildClass::type ? true : nameOfParentClass::IsTypeOf(t);                     \
}                                                                                                   \

