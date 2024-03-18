//
// Clark Kromenaker
//
// TypeInfo is a static member of a class that provides RTTI info for that class.
//
#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "TypeId.h"
#include "VariableInfo.h"

// To support dynamic construction via TypeInfo, we need to know a constructor is available.
// The default constructor could work, but it forces all classes with TypeInfo to have a default constructor, which is sometimes ambiguous/dangerous.
// Instead, we'll define a special constructor (via a unique argument) that this system calls to constuct objects dynamically. 
//typedef uint32_t DynamicTypeCtor;

class TypeInfo
{
public:
    TypeInfo(const char* typeName, TypeId typeId);
    virtual ~TypeInfo() = default;

    // Type
    const char* GetTypeName() const { return mTypeName; }
    TypeId GetTypeId() const { return mTypeId; }

    virtual TypeInfo* GetBaseType() const = 0;
    virtual bool IsTypeOf(TypeId typeId) const = 0;

    //virtual void* New() const = 0;
    //template<typename T> T* New() { return static_cast<T*>(New()); }

    // Type Variables
    void AddVariable(VariableType type, const char* name, size_t offset) { mVariables.emplace_back(type, name, offset); }

    std::vector<VariableInfo>& GetVariables() { return mVariables; }
    VariableInfo* GetVariableByName(const char* name);

    // Type Functions
    //void AddFunction(const std::string& name, void* funcPtr) { mFunctionMap[name] = funcPtr; }
    //template<typename T> T GetFunction(const std::string& name) { static_cast<T>(mFunctionMap[name]); }

private:
    // The name of the type.
    const char* mTypeName = nullptr;

    // A unique numeric identifier for this type.
    TypeId mTypeId = 0;

    // A list of all the variables for this type.
    std::vector<VariableInfo> mVariables;

    // Functions on this type that have been registered to be callable via a name/identifier.
    //std::unordered_map<std::string, void*> mFunctionMap;
};

// A "concrete" TypeInfo instances.
// All instances use this class, though most manipulation happens via the base class.
template<typename TClass, typename TBase>
class ConcreteTypeInfo : public TypeInfo
{
public:
    ConcreteTypeInfo(const char* typeName, TypeId typeId) : TypeInfo(typeName, typeId)
    {
        // We assume that the TypeInfo boilerplate defines a static init function for us to call.
        // Call it here to have the type do any additional custom work (like registering members).
        TClass::InitTypeInfo();
    }

    // Type
    TypeInfo* GetBaseType() const override
    {
        // If the base class is the "no base class" placeholder, return null to indicate we DON'T have a base type.
        if(TBase::sTypeInfo.GetTypeId() == NO_BASE_CLASS_TYPE_ID) { return nullptr; }
        return &TBase::sTypeInfo;
    }
    bool IsTypeOf(TypeId typeId) const override { return typeId == GetTypeId() || (GetTypeId() != NO_BASE_CLASS_TYPE_ID && TBase::sTypeInfo.IsTypeOf(typeId)); }

    //void* New() const override { return new TClass(); }

private:
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
    static ConcreteTypeInfo<NoBaseClass, NoBaseClass> sTypeInfo;
    static void InitTypeInfo();
};


//================
// TYPE INFO MACROS
//================
// Macros for generating boilerplate related to TypeInfos.
// A macro is helpful to ensure consistent naming and to generate any needed functions.

// Any class that wants runtime type info must add TYPEINFO.
#define TYPEINFO(PClass, PBaseClass) public: \
    static ConcreteTypeInfo<PClass, PBaseClass> sTypeInfo; \
    static void InitTypeInfo(); \
    virtual TypeInfo& GetTypeInfo() { return sTypeInfo; } \
    const char* GetTypeName() { return GetTypeInfo().GetTypeName(); } \
    TypeId GetTypeId() { return GetTypeInfo().GetTypeId(); } \
    template<typename pClass> bool IsA() { return GetTypeInfo().IsTypeOf(pClass::sTypeInfo.GetTypeId()); }
    

// Add this in cpp/impl to define static TypeInfo and start the InitType static function.
#define TYPEINFO_INIT(PClass, PBaseClass, PTypeId) \
ConcreteTypeInfo<PClass, PBaseClass> PClass::sTypeInfo(#PClass, PTypeId); \
void PClass::InitTypeInfo()

// Use this to add a variable to the class.
#define TYPEINFO_ADD_VAR(PClass, PType, PVar) sTypeInfo.AddVariable(PType, #PVar, offsetof(PClass, PVar));


//================
// TYPE INFO HELPER MACROS
//================
// Macros for easy access to type info accessors and queries.
#define TYPE_NAME_STATIC(PClass) PClass::sTypeInfo.GetTypeName()
#define TYPE_NAME(PInst) (PInst).GetTypeInfo().GetTypeName()

//#define TYPE_STATIC(PClass) PClass::sTypeInfo.GetTypeId()
//#define TYPE(PInst) (PInst).GetTypeInfo().GetTypeId()
// 
//#define IS_TYPE_OF(PInst, PClass) (PInst).GetTypeInfo().IsTypeOf(PClass::sTypeInfo.GetTypeId())

#define IS_CHILD_TYPE(PInst1, PInst2) (PInst1).GetTypeInfo().IsTypeOf((PInst2).GetTypeInfo().GetTypeId())
#define IS_SAME_TYPE(PInst1, PInst2) ((PInst1).GetTypeInfo().GetTypeId() == (PInst2).GetTypeInfo().GetTypeId())