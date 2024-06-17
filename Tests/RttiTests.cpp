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
        TYPEINFO_BASE(TestBaseClass);
    public:
        int mMyInt = 0;
        float mMyFloat = 0.0f;

        TestBaseClass() = default;
        void SimpleBaseClassFunc() { mMyInt++; }
        int ComplexBaseClassFunc(const std::vector<int>& stuff)
        {
            int sum = 0;
            for(int val : stuff)
            {
                sum += val;
            }
            return sum;
        }
    };
    TYPEINFO_INIT(TestBaseClass, NoBaseClass, 1)
    {
        TYPEINFO_VAR(TestBaseClass, VariableType::Int, mMyInt);
        TYPEINFO_VAR(TestBaseClass, VariableType::Float, mMyFloat);

        TYPEINFO_FUNC(TestBaseClass, SimpleBaseClassFunc);
        TYPEINFO_FUNC(TestBaseClass, ComplexBaseClassFunc);
    }

    class TestSubClass : public TestBaseClass
    {
        TYPEINFO_SUB(TestSubClass, TestBaseClass);
    public:
        std::string mMyString;

        TestSubClass() = default;
    };
    TYPEINFO_INIT(TestSubClass, TestBaseClass, 2)
    {
        TYPEINFO_VAR(TestSubClass, VariableType::String, mMyString);
    }
}

TEST_CASE("Type names are correct")
{
    // Static type name should match expectations.
    REQUIRE(strcmp(TestBaseClass::StaticTypeName(), "TestBaseClass") == 0);
    REQUIRE(strcmp(TestSubClass::StaticTypeName(), "TestSubClass") == 0);

    // Type name via an instance should match expectations.
    TestBaseClass b;
    REQUIRE(strcmp(b.GetTypeName(), "TestBaseClass") == 0);
    TestSubClass s;
    REQUIRE(strcmp(s.GetTypeName(), "TestSubClass") == 0);

    // Polymorphic type name checks should match expectations.
    TestBaseClass* basePtr = &s;
    REQUIRE(strcmp(basePtr->GetTypeName(), "TestSubClass") == 0);
}

TEST_CASE("Type IDs are correct")
{
    // Static type IDs should match expectations.
    REQUIRE(TestBaseClass::StaticTypeId() == 1);
    REQUIRE(TestSubClass::StaticTypeId() == 2);

    // Type IDs via an instance should match expectations.
    TestBaseClass b;
    REQUIRE(b.GetTypeId() == 1);
    TestSubClass s;
    REQUIRE(s.GetTypeId() == 2);

    // Polymorphic type IDs should match expectations.
    TestBaseClass* basePtr = &s;
    REQUIRE(basePtr->GetTypeId() == 2);
}

TEST_CASE("Type comparison is correct")
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

    // Correct type identification?
    TestBaseClass* basePtr = &s;
    REQUIRE(basePtr->IsA<TestSubClass>());
    REQUIRE(IS_SAME_TYPE(s, *basePtr));
}

TEST_CASE("Create dynamic instance works")
{
    // We have a subclass referenced through a base class pointer.
    TestSubClass s;
    TestBaseClass* basePtr = &s;

    // We should be able to create a new instance of TestSubClass via the base pointer.
    TestBaseClass* newInst = basePtr->GetTypeInfo().New<TestBaseClass>();
    REQUIRE(strcmp(newInst->GetTypeName(), "TestSubClass") == 0);
    REQUIRE(newInst->GetTypeId() == 2);
    REQUIRE(newInst->IsA<TestSubClass>());
    delete newInst;
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
}

TEST_CASE("Call function works")
{
    // Attempt to call a simple function on an instance, verify it has effect.
    TestBaseClass b;
    REQUIRE(b.mMyInt == 0);
    TestBaseClass::sTypeInfo.CallFunction<void>("SimpleBaseClassFunc", &b);
    REQUIRE(b.mMyInt == 1);

    // Call a more advanced function on an instance, verify that it works.
    std::vector<int> blah { 5, 10, 15, 20 };
    int result = TestBaseClass::sTypeInfo.CallFunction<int>("ComplexBaseClassFunc", &b, blah);
    REQUIRE(result == 50);
}