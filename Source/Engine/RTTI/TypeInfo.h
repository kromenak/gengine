//
// Clark Kromenaker
//
// TypeInfo provides information about a type and dynamic ways to perform operations on the type.
// Supported operations:
//  1) Retrieving a unique type ID that is persistent across compiles, runs, and versions of the program.
//  2) Retrieve the type name as a string.
//  3) Check and navigate a type's inheritance hierarchy.
//  4) Check a type's registered variables and even modify the variables on an instance.
//  5) Check a type's registered functions and even call a function on an instance.
//  6) Create new instances of a type dynamically.
//
#pragma once
#include <cstring>
#include <string>
#include <vector>

#include "TypeId.h"
#include "VariableInfo.h"

// Dynamic creation of an instance via TypeInfo requires a guaranteed available constructor.
// Default constructor could work, but is ambiguous (and what if its used for some other purpose?).
// Instead, we define a special constructor (via a unique DynamicCtor argument) that is used specifically for dynamic creation.
struct DynamicCtor { };

// A macro that hides the complex syntax of calling a member function pointer on an instance.
// Recommended by the C++ FAQ: https://isocpp.org/wiki/faq/pointers-to-members
#define CALL_MEMBER_FN(PObject, PMemberFuncPtr) ((PObject).*(PMemberFuncPtr))

// Abstract base class for TypeInfo.
// Not *exactly* an interface (it has data), but close enough.
class GTypeInfo
{
public:
    GTypeInfo(const char* typeName, TypeId typeId);
    virtual ~GTypeInfo() = default;

    bool operator==(const GTypeInfo& other) const { return other.mTypeId == mTypeId; }

    // Type
    const char* GetTypeName() const { return mTypeName; }
    TypeId GetTypeId() const { return mTypeId; }

    // Type Hierarchy
    virtual GTypeInfo* GetBaseType() const = 0;
    virtual bool IsTypeOf(TypeId typeId) const = 0;

    // Type Instances
    virtual void* New() const = 0;
    template<typename T> T* New() { return static_cast<T*>(New()); }

    // Type Variables
    void AddVariable(VariableType type, const char* name, size_t offset) { mVariables.emplace_back(type, name, offset); }
    std::vector<VariableInfo>& GetVariables() { return mVariables; }
    VariableInfo* GetVariableByName(const char* name);

private:
    // The name of the type.
    const char* mTypeName = nullptr;

    // A unique numeric identifier for this type.
    TypeId mTypeId = 0;

    // Registered variables for this type.
    std::vector<VariableInfo> mVariables;
};

// A "concrete" TypeInfo. All types create instances of this class, though some manipulation happens via the base class.
template<typename TClass, typename TBase>
class TypeInfo : public GTypeInfo
{
public:
    TypeInfo(const char* typeName, TypeId typeId) : GTypeInfo(typeName, typeId)
    {
        // We assume that the TypeInfo boilerplate defines a static init function for us to call.
        // Call it here to have the type do any additional custom work (like registering member variables/functions).
        TClass::InitTypeInfo();
    }

    // Type Hierarchy
    GTypeInfo* GetBaseType() const override
    {
        // If the base class is the "no base class" placeholder, return null to indicate we DON'T have a base type.
        if(TBase::sTypeInfo.GetTypeId() == NO_BASE_CLASS_TYPE_ID) { return nullptr; }
        return &TBase::sTypeInfo;
    }
    bool IsTypeOf(TypeId typeId) const override { return typeId == GetTypeId() || (GetTypeId() != NO_BASE_CLASS_TYPE_ID && TBase::sTypeInfo.IsTypeOf(typeId)); }

    // Type Instances
    void* New() const override { return new TClass(DynamicCtor()); }
    TClass* New() { return static_cast<TClass*>(New()); }

    // Type Functions
    typedef void(TClass::*MemberFunc)();  // use typedef and template to define function ptr type that is a member of this type.
    void AddFunction(const char* name, MemberFunc func) { mFunctions.emplace_back(name, func); }
    template<typename TRet, typename... Args> TRet CallFunction(const char* name, TClass* instance, Args&&... args)
    {
        // Find the function and call it.
        for(Function& function : mFunctions)
        {
            if(strcmp(function.name, name) == 0)
            {
                // Need to convert general "TClass void* function" into correct function signature before calling.
                // FORTUNATELY...C++ templates DO support this...via some pretty involved syntax...
                TRet(TClass::*func)(Args...) = (TRet(TClass::*)(Args...))function.func;
                return static_cast<TRet>(CALL_MEMBER_FN(*instance, func)(args...));
            }
        }
        return TRet();
    }

private:
    // Registered functions for this type.
    // All functions are stored essentially as "void*", so they need to be converted to the correct signature before calling them.
    // There is no checking, so calling a function pointer with the wrong signature will probably crash your program!
    struct Function
    {
        const char* name = nullptr;
        MemberFunc func = nullptr;
        Function(const char* name, MemberFunc func) : name(name), func(func) { }
    };
    std::vector<Function> mFunctions;

    // All instances of this type.
    //std::vector<TClass*> mInstances;
};


//=======================
// PLACEHOLDER BASE CLASS
//=======================
// Acts as a "stand-in" base class when no base class is needed.
// This reduces logic variants needed in the TypeInfo class (if we can just assume all classes have a base class).
class NoBaseClass
{
public:
    static TypeInfo<NoBaseClass, NoBaseClass> sTypeInfo;
    static void InitTypeInfo();
    NoBaseClass(DynamicCtor ctor) { }
};


//================
// TYPE INFO MACROS
//================
// Macros for generating boilerplate related to TypeInfos.
// A macro is helpful to ensure consistent naming and to generate any needed functions.
#define INTERNAL_TYPEINFO_CORE(PClass, PBaseClass) public: \
    typedef TypeInfo<PClass, PBaseClass> TypeInfoType; \
    static TypeInfo<PClass, PBaseClass> sTypeInfo; \
    static void InitTypeInfo(); \

#define INTERNAL_TYPEINFO_MEMBERFUNCS_STATIC() static const char* StaticTypeName() { return sTypeInfo.GetTypeName(); } \
    static TypeId StaticTypeId() { return sTypeInfo.GetTypeId(); }

#define INTERNAL_TYPEINFO_MEMBERFUNCS() GTypeInfo& GetTypeInfo() { return sTypeInfo; } \
    const char* GetTypeName() { return GetTypeInfo().GetTypeName(); } \
    TypeId GetTypeId() { return GetTypeInfo().GetTypeId(); } \
    template<typename pClass> bool IsA() { return GetTypeInfo().IsTypeOf(pClass::sTypeInfo.GetTypeId()); } \
    INTERNAL_TYPEINFO_MEMBERFUNCS_STATIC()

// For basic classes with no inheritance. No polymorphism.
#define TYPEINFO(PClass) INTERNAL_TYPEINFO_CORE(PClass, NoBaseClass) \
    INTERNAL_TYPEINFO_MEMBERFUNCS() \
    PClass(DynamicCtor ctor) { }

// For base classes with inheritance hierarchies. Polymorphism included (see "virtual" keyword).
#define TYPEINFO_BASE(PClass) INTERNAL_TYPEINFO_CORE(PClass, NoBaseClass) \
    virtual INTERNAL_TYPEINFO_MEMBERFUNCS() \
    PClass(DynamicCtor ctor) { }

// For subclasses in inheritance hierarchies. Polymorphic, and calls base class's special constructor.
#define TYPEINFO_SUB(PClass, PBaseClass) INTERNAL_TYPEINFO_CORE(PClass, PBaseClass) \
    virtual INTERNAL_TYPEINFO_MEMBERFUNCS() \
    PClass(DynamicCtor ctor) : PBaseClass(ctor) { }

// Add this in cpp/impl to define static TypeInfo and start the InitType static function.
#define TYPEINFO_INIT(PClass, PBaseClass, PTypeId) \
TypeInfo<PClass, PBaseClass> PClass::sTypeInfo(#PClass, PTypeId); \
void PClass::InitTypeInfo()

// A variant used for templatized types in the type hierarchy.
//TODO: This uses typeid(T).name() to ensure unique type names and IDs. However, this value is not consistent across compilers.
//TODO: If type names or IDs are ever needed in serialized data, this would need to change.
#define TYPEINFO_INIT_TEMPLATE(PClass, PBaseClass) \
template<typename T> TypeInfo<PClass<T>, PBaseClass> PClass<T>::sTypeInfo((std::string(#PClass) + typeid(T).name()).c_str(), GENERATE_TYPE_ID); \
template<typename T> void PClass<T>::InitTypeInfo() { }

//#define TYPEINFO_INIT(PClass, PBaseClass) \
//TypeInfo<PClass, PBaseClass> PClass::sTypeInfo(#PClass, GENERATE_TYPE_ID); \
//void PClass::InitTypeInfo()

// Use this to register a variable to the type's TypeInfo.
#define TYPEINFO_VAR(PClass, PType, PVar) sTypeInfo.AddVariable(PType, #PVar, offsetof(PClass, PVar))

// Use this to register a function to the type's TypeInfo.
#define TYPEINFO_FUNC(PClass, PFunc) sTypeInfo.AddFunction(#PFunc, (PClass::TypeInfoType::MemberFunc)&PClass::PFunc)

//================
// TYPE INFO HELPER MACROS
//================
// Macros for easy access to type info accessors and queries.
#define IS_CHILD_TYPE(PInst1, PInst2) (PInst1).GetTypeInfo().IsTypeOf((PInst2).GetTypeInfo().GetTypeId())
#define IS_SAME_TYPE(PInst1, PInst2) ((PInst1).GetTypeInfo() == (PInst2).GetTypeInfo())