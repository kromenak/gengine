#include "SheepThread.h"

#include <iostream>

#include "SheepVM.h"

std::string SheepThread::GetName() const
{
	if(mContext != nullptr)
	{
		return mContext->GetName() + ":" + mFunctionName;
	}
	return ":" + mFunctionName;
}

std::function<void()> SheepThread::AddWait()
{
    if(!mInWaitBlock) { return nullptr; }

    ++mWaitCounter;
    //std::cout << GetName() << " added wait (" << mWaitCounter << " waits total)" << std::endl;
    return std::bind(&SheepThread::OnWaitCompleted, this);
}

void SheepThread::OnWaitCompleted()
{
	assert(mInWaitBlock);
	assert(mWaitCounter > 0);
	--mWaitCounter;
    //std::cout << GetName() << " removed wait (" << mWaitCounter << " waits remain)" << std::endl;

	if(mBlocked && mWaitCounter == 0)
	{
		mVirtualMachine->ExecuteInternal(this);
	}
}
