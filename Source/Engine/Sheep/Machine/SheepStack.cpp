#include "SheepStack.h"

#include <iostream>

#include "PersistState.h"

//#define SHEEP_DEBUG

void SheepStack::PushInt(int val)
{
    mStackSize++;
    assert(mStackSize < kMaxStackSize);

    mStack[mStackSize - 1].type = SheepValueType::Int;
    mStack[mStackSize - 1].intValue = val;

    #ifdef SHEEP_DEBUG
    std::cout << "SHEEP STACK: Push 1 (Stack Size = " << mStackSize << ")" << std::endl;
    #endif
}

void SheepStack::PushFloat(float val)
{
    mStackSize++;
    assert(mStackSize < kMaxStackSize);

    mStack[mStackSize - 1].type = SheepValueType::Float;
    mStack[mStackSize - 1].floatValue = val;

    #ifdef SHEEP_DEBUG
    std::cout << "SHEEP STACK: Push 1 (Stack Size = " << mStackSize << ")" << std::endl;
    #endif
}

void SheepStack::PushStringOffset(int val)
{
    mStackSize++;
    assert(mStackSize < kMaxStackSize);

    mStack[mStackSize - 1].type = SheepValueType::String;
    mStack[mStackSize - 1].intValue = val;

    #ifdef SHEEP_DEBUG
    std::cout << "SHEEP STACK: Push 1 (Stack Size = " << mStackSize << ")" << std::endl;
    #endif
}

void SheepStack::PushString(const char* str)
{
    mStackSize++;
    assert(mStackSize < kMaxStackSize);

    mStack[mStackSize - 1].type = SheepValueType::String;
    mStack[mStackSize - 1].stringValue = str;

    #ifdef SHEEP_DEBUG
    std::cout << "SHEEP STACK: Push 1 (Stack Size = " << mStackSize << ")" << std::endl;
    #endif
}

SheepValue& SheepStack::Pop()
{
    SheepValue& top = mStack[mStackSize - 1];
    mStackSize--;

    #ifdef SHEEP_DEBUG
    std::cout << "SHEEP STACK: Pop 1 (Stack Size = " << mStackSize << ")" << std::endl;
    #endif
    assert(mStackSize >= 0); // Or clamp?

    return top;
}

void SheepStack::Pop(int count)
{
    mStackSize -= count;

    #ifdef SHEEP_DEBUG
    std::cout << "SHEEP STACK: Pop " << count << " (Stack Size = " << mStackSize << ")" << std::endl;
    #endif
    assert(mStackSize >= 0); // Or clamp?
}

void SheepStack::OnPersist(PersistState& ps)
{
    // This is a bit of a HACK, but deals with the problem of storing loaded String stack values.
    // String stack values are "const char*" - so the pointed to memory must exist somewhere!
    // Usually, these are literals from the sheep script program.
    // But since we're loading from disk, loaded strings must be cached somewhere - this is the place.
    static std::vector<std::string> strCache;
    if(ps.IsLoading())
    {
        strCache.clear();
    }

    // Save/load the stack size.
    ps.Xfer(PERSIST_VAR(mStackSize));

    // Save/load each stack type/value.
    for(int i = 0; i < mStackSize; ++i)
    {
        ps.Xfer<SheepValueType, int>("", mStack[i].type);
        switch(mStack[i].type)
        {
            case SheepValueType::Void:
                break;
            case SheepValueType::Int:
                ps.Xfer("", mStack[i].intValue);
                break;
            case SheepValueType::Float:
                ps.Xfer("", mStack[i].floatValue);
                break;
            case SheepValueType::String:
                // When saving, this saves the string out.
                std::string str(mStack[i].stringValue);
                ps.Xfer("", str);

                // When loading, store the loaded string in the cache.
                // And then store the pointer to the c-str in the stack value.
                if(ps.IsLoading())
                {
                    strCache.push_back(str);
                    mStack[i].stringValue = strCache.back().c_str();
                }
                break;
        }
    }
}