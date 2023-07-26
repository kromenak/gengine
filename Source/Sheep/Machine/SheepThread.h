//
// Clark Kromenaker
//
// Sheep is (currently) single-threaded. But each "thread" represents a
// different bytecode sequence at a different state of execution.
//
#pragma once
#include <functional>
#include <string>

#include "SheepStack.h"

class SheepVM;
struct SheepInstance;

struct SheepThread
{
	// Reference to this thread's virtual machine.
	SheepVM* mVirtualMachine = nullptr;
	
	// The sheep instance attached to this thread.
    // Encapsulates SheepScript to run, as well as current variable state.
	SheepInstance* mContext = nullptr;
	
	// Each thread has its own stack.
	SheepStack mStack;
	
	// Current code offset for attached sheep (aka the instruction pointer).
	int mCodeOffset = 0;
	
	// Info about the function being executed (mainly for debugging).
	std::string mFunctionName;
	int mFunctionStartOffset = 0;
	
	// If true, this thread is executing the attached sheep.
	// This is still "true" while "waiting" on something else.
	bool mRunning = false;

    // If true, the thread is currently blocked.
    // This happens when we're in a wait block, but not all waitable things have returned yet.
	bool mBlocked = false;
	
	// If set, some other thread is waiting for this thread to complete before continuing.
	// Could happen if some sheep "waits" on a system function.
	std::function<void()> mWaitCallback = nullptr;
	
	// The number of "things" this thread is waiting on.
	// When exiting a wait block, thread execution will be blocked until this counter reaches zero.
	int mWaitCounter = 0;
	
	// If true, the thread is executing code inside a wait block.
	// Before exiting the wait block, all waited upon functions must complete.
	bool mInWaitBlock = false;

    // A tag allows executing threads to be identified/categorized/grouped. Currently used to stop a set of threads prematurely.
    // Tags are inherited - if a thread starts another thread, that "child" thread gets the same tag.
    // (this is similar to the "owning layer" in the original game, but a bit more generalized)
    std::string mTag;

    void Reset();

	std::string GetName() const;

    std::function<void()> AddWait();
    void RemoveWait();
    
	//TODO
	// owning layer?
	// debug info?
	// is preloading only?
};

