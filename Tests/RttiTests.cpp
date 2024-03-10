//
// Clark Kromenaker
//
// Tests for RTTI and type info systems.
//
#include "catch.hh"

#include "TypeInfo.h"

namespace
{
    // A sample class hierarchy used to test RTTI.
    class TestBaseClass
    {
        TYPEINFO(TestBaseClass, NoBaseClass);
    public:
        int mMyInt = 0;
        float mMyFloat = 0.0f;
    };
    TYPEINFO_INIT(TestBaseClass, NoBaseClass, 1)
    {
        TYPEINFO_ADD_VAR(TestBaseClass, Int, mMyInt);
        TYPEINFO_ADD_VAR(TestBaseClass, Float, mMyFloat);
    }

    class TestSubClass : public TestBaseClass
    {
        TYPEINFO(TestSubClass, TestBaseClass);
    public:
        std::string mMyString;
    };
    TYPEINFO_INIT(TestSubClass, TestBaseClass, 2)
    {
        TYPEINFO_ADD_VAR(TestSubClass, String, mMyString);
    }
}

// The tests...
TEST_CASE("Type Names are correct")
{
    // Make sure the type names returned match expectations.
    TestBaseClass b;
    REQUIRE(strcmp(TYPE_NAME_STATIC(TestBaseClass), "TestBaseClass") == 0);
    REQUIRE(strcmp(TYPE_NAME(b), "TestBaseClass") == 0);

    TestSubClass s;
    REQUIRE(strcmp(TYPE_NAME_STATIC(TestSubClass), "TestSubClass") == 0);
    REQUIRE(strcmp(TYPE_NAME(s), "TestSubClass") == 0);
}

TEST_CASE("Type IDs are correct")
{
    // Make sure the type IDs returned match expectations.
    TestBaseClass b;
    REQUIRE(TestBaseClass::sTypeInfo.GetTypeId() == 1);
    REQUIRE(b.GetTypeId() == 1);
    
    TestSubClass s;
    REQUIRE(TestSubClass::sTypeInfo.GetTypeId() == 2);
    REQUIRE(s.GetTypeId() == 2);
}

TEST_CASE("Type checking is correct")
{
    // A base class should be identified as its own type, but not a subclass type.
    TestBaseClass b;
    REQUIRE(b.IsA<TestBaseClass>());
    REQUIRE(!b.IsA<TestSubClass>());

    // The same object should be considered to have the same type.
    REQUIRE(IS_SAME_TYPE(b, b));

    // Two different class instances should say they have the same type.
    TestBaseClass b2;
    REQUIRE(IS_SAME_TYPE(b, b2));

    // A subclass should be identified as its type and any base class type.
    TestSubClass s;
    REQUIRE(s.IsA<TestBaseClass>());
    REQUIRE(s.IsA<TestSubClass>());

    // A subclass and a base class are not the same type.
    REQUIRE(!IS_SAME_TYPE(b, s));

    // But the subclass is a child type of the base type.
    REQUIRE(IS_CHILD_TYPE(s, b));

    // But not the other way around!
    REQUIRE(!IS_CHILD_TYPE(b, s));
}

TEST_CASE("Polymorphic type info works")
{
    // Let's say we have an instance of a subclass, but it is referenced through a base class pointer or reference...
    TestSubClass s;
    TestBaseClass& baseRef = s;
    TestBaseClass* basePtr = &s;

    // Do we still get the correct type name via the ref or ptr?
    REQUIRE(strcmp(TYPE_NAME(baseRef), "TestSubClass") == 0);
    REQUIRE(strcmp(TYPE_NAME(*basePtr), "TestSubClass") == 0);

    // Correct type ID?
    REQUIRE(baseRef.GetTypeId() == 2);
    REQUIRE(basePtr->GetTypeId() == 2);

    // Correct type identification?
    REQUIRE(baseRef.IsA<TestSubClass>());
    REQUIRE(IS_SAME_TYPE(baseRef, *basePtr));
}

TEST_CASE("Create instance from Type works")
{
    /*
    // We have a subclass referenced through a base class pointer.
    TestSubClass s;
    TestBaseClass* basePtr = &s;

    // We should be able to create a new instance of TestSubClass via the base pointer.
    void* newInst = basePtr->GetTypeInfo().New();
    TestBaseClass* newBasePtr = static_cast<TestBaseClass*>(newInst);
    REQUIRE(strcmp(TYPE_NAME(*newBasePtr), "TestSubClass") == 0);
    REQUIRE(TYPE(*newBasePtr) == 2);
    REQUIRE(IS_TYPE_OF(*newBasePtr, TestSubClass));
    delete newBasePtr;

    // Same thing, but more streamlined via templated New<> function.
    TestSubClass* subPtr = basePtr->GetTypeInfo().New<TestSubClass>();
    REQUIRE(strcmp(TYPE_NAME(*subPtr), "TestSubClass") == 0);
    REQUIRE(TYPE(*subPtr) == 2);
    REQUIRE(IS_TYPE_OF(*subPtr, TestSubClass));
    delete subPtr;
    */
}

TEST_CASE("Modify variable in immediate class works")
{
    TestSubClass s;

    // Retrieve variable info from instance. Should be retrievable.
    VariableInfo* myString = s.GetTypeInfo().GetVariableByName("mMyString");
    REQUIRE(myString != nullptr);

    // Modify variable indirectly. Value should update as expected.
    myString->GetRef<std::string>(&s) = "Hello";
    REQUIRE(s.mMyString == "Hello");


    /*
    REQUIRE(s.mMyInt == 0);

    // Retrieve myInt class variable from subclass instance. Should be retrievable.
    VariableInfo* myInt = s.GetTypeInfo().GetVariableByName("mMyInt");
    REQUIRE(myInt != nullptr);

    // Modify myInt via RTTI and verify it changed as expected.
    myInt->GetRef<int>(&s) = 10;
    REQUIRE(s.mMyInt == 10);
    */
}