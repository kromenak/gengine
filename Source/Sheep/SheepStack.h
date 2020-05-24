//
// SheepStack.h
//
// Clark Kromenaker
//
// A stack of Sheep values.
//
#pragma once

#include "SheepValue.h"

struct SheepStack
{
public:
	void PushInt(int val);
	void PushFloat(float val);
	void PushStringOffset(int val);
	void PushString(const char* str);
	
	SheepValue& Peek() { assert(mStackSize > 0); return mStack[mStackSize - 1]; }
	SheepValue& Peek(int index) { assert(mStackSize > 0 && index < mStackSize); return mStack[mStackSize - 1 - index]; }
	SheepValue& Pop();
	void Pop(int count);
	
	int Size() const { return mStackSize; }
	void Clear() { mStackSize = 0; }
	
private:
	static const int kMaxStackSize = 1024;
	int mStackSize = 0;
	SheepValue mStack[kMaxStackSize];
};
