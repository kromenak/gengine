#include "SheepThread.h"

#include <iostream>

#include "SheepVM.h"

void SheepThread::Reset()
{
    mContext = nullptr;

    mStack.Clear();

    mCodeOffset = 0;

    mFunctionName.clear();
    mFunctionStartOffset = 0;

    mRunning = false;
    mBlocked = false;

    mWaitCallback = nullptr;
    mWaitCounter = 0;
    mInWaitBlock = false;

    mTag.clear();
}

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

    NotifyLink* notifyLink = mVirtualMachine->GetNotifyLink();
    notifyLink->thread = this;
    return notifyLink->AddNotify();
    //return std::bind(&SheepThread::OnWaitCompleted, this);
}

void SheepThread::RemoveWait()
{
    assert(mInWaitBlock);
    assert(mWaitCounter > 0);
    --mWaitCounter;
    //std::cout << GetName() << " removed wait (" << mWaitCounter << " waits remain)" << std::endl;

    if(mBlocked && mWaitCounter == 0)
    {
        mVirtualMachine->ContinueExecution(this);
    }
}