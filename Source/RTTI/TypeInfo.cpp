#include "TypeInfo.h"

#include <cstring>

#include "StringUtil.h"
#include "TypeDatabase.h"

TYPEINFO_INIT(NoBaseClass, NoBaseClass, NO_BASE_CLASS_TYPE_ID)
{

}

GTypeInfo::GTypeInfo(const char* typeName, TypeId typeId) :
    mTypeName(typeName),
    mTypeId(typeId)
{
    // If an invalid type ID was passed in, we'll generate one.
    if(typeId == GENERATE_TYPE_ID)
    {
        mTypeId = StringUtil::Hash(typeName);
    }

    // Register this type in the database.
    TypeDatabase::Get().RegisterType(this);
}

VariableInfo* GTypeInfo::GetVariableByName(const char* name)
{
    // Find a variable with this name.
    for(VariableInfo& variable : mVariables)
    {
        if(strcmp(variable.GetName(), name) == 0)
        {
            return &variable;
        }
    }

    // Can't find any variable with that name on this Type!
    return nullptr;
}