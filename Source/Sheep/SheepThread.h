//
// SheepThread.h
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
class SheepInstance;

struct SheepThread
{
	// Reference to this thread's virtual machine.
	SheepVM* mVirtualMachine = nullptr;
	
	// The sheep attached to this thread.
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
	
	std::string GetName() const;
	
	std::function<void()> AddWait()
	{
		if(!mInWaitBlock) { return nullptr; }
		mWaitCounter++;
		return std::bind(&SheepThread::OnWaitCompleted, this);
	}
	
private:
	void OnWaitCompleted();
	
	//TODO
	// owning layer?
	// int mFunctionOffset = 0;
	// debug info?
	// is preloading only?
	// bool mIsCurrentlyBlocking = false;
};
