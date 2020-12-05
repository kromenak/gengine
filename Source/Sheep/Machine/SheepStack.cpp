//
// SheepStack.cpp
//
// Clark Kromenaker
//
#include "SheepStack.h"

#include <iostream>

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
