//
// SheepThread.cpp
//
// Clark Kromenaker
//
#include "SheepThread.h"

#include "SheepVM.h"

std::string SheepThread::GetName() const
{
	if(mContext != nullptr)
	{
		return mContext->GetName() + ":" + mFunctionName;
	}
	return ":" + mFunctionName;
}

void SheepThread::OnWaitCompleted()
{
	assert(mInWaitBlock);
	assert(mWaitCounter > 0);
	mWaitCounter--;
	if(mBlocked && mWaitCounter == 0)
	{
		mVirtualMachine->ExecuteInternal(this);
	}
}
