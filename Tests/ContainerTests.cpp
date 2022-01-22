//
// Clark Kromenaker
//
// Tests for custom container classes.
//
#include "catch.hh"

#include "Queue.h"
#include "ResizableQueue.h"
#include "Stack.h"

// Helper object to store in containers.
// A bit more interesting to test than built-in types.
struct TestObject
{
    TestObject(int value) : value(value)
    {
        printf("Construct\n");
    }
    ~TestObject()
    {
        printf("Destruct\n");
    }
    int value = 0;
};

TEST_CASE("Queue (fixed size) works")
{
    // Test initial state.
    Queue<TestObject, 10> queue;
    REQUIRE(queue.Size() == 0);
    REQUIRE(queue.Empty());
    REQUIRE(queue.Capacity() == 10);

    // Test pushing.
    for(int i = 0; i < 5; ++i)
    {
        queue.Push(TestObject(i));
    }
    for(int i = 0; i < 5; ++i)
    {
        REQUIRE(queue[i].value == i);
    }
    REQUIRE(queue.Size() == 5);
    REQUIRE(!queue.Empty());
    REQUIRE(queue.Capacity() == 10);
    REQUIRE(queue.Front().value == 0);
    REQUIRE(queue.Back().value == 4);

    // Test popping.
    for(int i = 0; i < 3; ++i)
    {
        REQUIRE(queue.Front().value == i);
        queue.Pop();
    }
    REQUIRE(queue.Size() == 2);
    REQUIRE(!queue.Empty());
    REQUIRE(queue.Capacity() == 10);
    REQUIRE(queue.Front().value == 3);
    REQUIRE(queue.Back().value == 4);

    // After popping, remaining values should be as expected.
    REQUIRE(queue[0].value == 3);
    REQUIRE(queue[1].value == 4);

    // Clear the queue.
    queue.Clear();
    REQUIRE(queue.Size() == 0);
    REQUIRE(queue.Empty());

    // Fill it up (use emplace to test that out).
    // At this point, the head/tail should be such that filling it up causes the tail to wrap around.
    for(int i = 0; i < queue.Capacity(); ++i)
    {
        queue.Emplace(i);
    }
    REQUIRE(queue.Size() == queue.Capacity());
    for(int i = 0; i < queue.Capacity(); ++i)
    {
        REQUIRE(queue[i].value == i);
    }

    // Test out range-based for support.
    int expectedVal = 0;
    for(auto element : queue)
    {
        REQUIRE(element.value == expectedVal);
        ++expectedVal;
    }
    REQUIRE(expectedVal == queue.Capacity());

    // Now pop each element off. Again, the head should end up wrapping around.
    for(int i = 0; i < queue.Capacity(); ++i)
    {
        REQUIRE(queue.Front().value == i);
        queue.Pop();
    }
    REQUIRE(queue.Empty());
}

TEST_CASE("Queue (resizable) works")
{
    // Test initial state.
    ResizableQueue<int> queue(10);
    REQUIRE(queue.Size() == 0);
    REQUIRE(queue.Empty());
    REQUIRE(queue.Capacity() == 10);

    // Test pushing.
    for(int i = 0; i < 5; ++i)
    {
        queue.Push(i);
    }
    for(int i = 0; i < 5; ++i)
    {
        REQUIRE(queue[i] == i);
    }
    REQUIRE(queue.Size() == 5);
    REQUIRE(!queue.Empty());
    REQUIRE(queue.Capacity() == 10);
    REQUIRE(queue.Front() == 0);
    REQUIRE(queue.Back() == 4);

    // Test popping.
    for(int i = 0; i < 3; ++i)
    {
        REQUIRE(queue.Front() == i);
        queue.Pop();
    }
    REQUIRE(queue.Size() == 2);
    REQUIRE(!queue.Empty());
    REQUIRE(queue.Capacity() == 10);
    REQUIRE(queue.Front() == 3);
    REQUIRE(queue.Back() == 4);

    // After popping, remaining values should be as expected.
    REQUIRE(queue[0] == 3);
    REQUIRE(queue[1] == 4);

    // Clear the queue.
    queue.Clear();
    REQUIRE(queue.Size() == 0);
    REQUIRE(queue.Empty());

    // Fill it up.
    // At this point, the head/tail should be such that filling it up causes the tail to wrap around.
    for(int i = 0; i < queue.Capacity(); ++i)
    {
        queue.Push(i);
    }
    REQUIRE(queue.Size() == queue.Capacity());
    for(int i = 0; i < queue.Capacity(); ++i)
    {
        REQUIRE(queue[i] == i);
    }

    // Ok, let's add TOO MANY elements - this should cause a container resize.
    for(int i = 10; i < 20; ++i)
    {
        queue.Push(i);
    }
    REQUIRE(queue.Size() == 20);
    REQUIRE(queue.Capacity() == 20);
    REQUIRE(queue[0] == 0);
    REQUIRE(queue.Back() == 19);

    // Test out range-based for support.
    int expectedVal = 0;
    for(auto element : queue)
    {
        REQUIRE(element == expectedVal);
        ++expectedVal;
    }
    REQUIRE(expectedVal == queue.Capacity());

    // Now pop each element off. Again, the head should end up wrapping around.
    for(int i = 0; i < queue.Capacity(); ++i)
    {
        REQUIRE(queue.Front() == i);
        queue.Pop();
    }
    REQUIRE(queue.Empty());
}

TEST_CASE("Stack (fixed size) works")
{
    Stack<TestObject, 10> stack;
}