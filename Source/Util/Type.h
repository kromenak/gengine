//
// Clark Kromenaker
//
// Defines a "Type" and macros for adding runtime type info (RTTI) to a base class and subclasses.
//
#include <cstddef>
#include <functional>   // for std::hash support
#include <string>

// A class's type will just be a size_t alias for now.
typedef std::size_t Type;

// Converts passed in symbol to a string (MyClass => "MyClass").
#define SYMBOL_TO_STR(x) #x

// Generates a Type for a particular symbol.
// Allows a "Type" to be generated for *any* class, not just those that use TYPE_DECL/DEF macros below.
// Note: std::hash will not necessarily return the same value across runs of a program!
#define GENERATE_TYPE(x) std::hash<std::string>()(SYMBOL_TO_STR(x));

// USAGE: Add TYPE_DECL* macro inside the class declaration.
// USAGE: Add TYPE_DEF* macro inside the class implementation file.

//////////////////////////////////////////////
// Use this version for classes that DO NOT have children.
#define TYPE_DECL()                                                 \
private:                                                            \
    static const Type type;                                         \
    static const char* typeName;                                    \
                                                                    \
public:                                                             \
    static Type GetType() { return type; }                          \
    static const char* GetTypeName() { return typeName; }           \
    bool IsTypeOf(const Type t) const { return t == type; }

#define TYPE_DEF(nameOfClass)                                                         \
const Type nameOfClass::type = std::hash<std::string>()(SYMBOL_TO_STR(nameOfClass));  \
const char* nameOfClass::typeName = SYMBOL_TO_STR(nameOfClass);
//////////////////////////////////////////////

//////////////////////////////////////////////
// Use this version for base classes that DO have children.
#define TYPE_DECL_BASE()                                            \
private:                                                            \
    static const Type type;                                         \
                                                                    \
public:                                                             \
    static Type GetType() { return type; }                          \
    virtual bool IsTypeOf(const Type t) const { return t == type; }

#define TYPE_DEF_BASE(nameOfClass)                                                    \
const Type nameOfClass::type = std::hash<std::string>()(SYMBOL_TO_STR(nameOfClass));
//////////////////////////////////////////////

//////////////////////////////////////////////
// Use this version for child classes.
#define TYPE_DECL_CHILD()                                   \
private:                                                    \
    static const Type type;                                 \
                                                            \
public:                                                     \
    static Type GetType() { return type; }                  \
    virtual bool IsTypeOf(const Type t) const override;

#define TYPE_DEF_CHILD(nameOfParentClass, nameOfChildClass)                                         \
const Type nameOfChildClass::type = std::hash<std::string>()(SYMBOL_TO_STR(nameOfChildClass));      \
                                                                                                    \
bool nameOfChildClass::IsTypeOf(const Type t) const {                                               \
    return t == nameOfChildClass::type ? true : nameOfParentClass::IsTypeOf(t);                     \
}
//////////////////////////////////////////////