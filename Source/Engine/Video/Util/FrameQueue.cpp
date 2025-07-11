#include "FrameQueue.h"

#include "PacketQueue.h"

Frame::Frame()
{
    frame = av_frame_alloc();
    if(frame == nullptr)
    {
        av_log(nullptr, AV_LOG_FATAL, "Could not allocate AVFrame!");
    }

    // Clear memory for these structs.
    memset(&sub, 0, sizeof(AVSubtitle));
    memset(&aspectRatio, 0, sizeof(AVRational));
}

Frame::~Frame()
{
    Unref();
    av_frame_free(&frame);
}

void Frame::Unref()
{
    av_frame_unref(frame);
    avsubtitle_free(&sub);
}

int FrameQueue::Init(PacketQueue* packetQueue, int maxSize, bool keepLast)
{
    mPacketQueue = packetQueue;
    mMaxSize = FFMIN(maxSize, FRAME_QUEUE_SIZE);
    mKeepLastFrame = keepLast;

    // Create mutex or fail.
    mMutex = SDL_CreateMutex();
    if(mMutex == nullptr)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }

    // Create condition or fail.
    mSizeChangedCondition = SDL_CreateCond();
    if(mSizeChangedCondition == nullptr)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    return 0;
}

void FrameQueue::Destroy()
{
    SDL_DestroyMutex(mMutex);
    SDL_DestroyCond(mSizeChangedCondition);
}

void FrameQueue::Enqueue()
{
    // Increment write index (with wraparound).
    ++mWriteIndex;
    if(mWriteIndex == mMaxSize)
    {
        mWriteIndex = 0;
    }

    // Increment size and signal to anyone waiting in Peek functons that they should try again.
    SDL_LockMutex(mMutex);
    mSize++;
    SDL_CondSignal(mSizeChangedCondition);
    SDL_UnlockMutex(mMutex);
}

void FrameQueue::Dequeue()
{
    // If want to always keep the last readable frame, basically just skip
    // the first call to "Next()" so that all subsequent calls are one behind.
    if(mKeepLastFrame && mReadIndexOffset == 0)
    {
        mReadIndexOffset = 1;
        return;
    }

    // Clear refs for current read frame.
    // This allows it to be used for writing.
    mQueue[mReadIndex].Unref();

    // Increment read index (with wraparound).
    ++mReadIndex;
    if(mReadIndex == mMaxSize)
    {
        mReadIndex = 0;
    }

    // Decrement size and signal to anyone waiting in peek functions to try again.
    SDL_LockMutex(mMutex);
    mSize--;
    SDL_CondSignal(mSizeChangedCondition);
    SDL_UnlockMutex(mMutex);
}

Frame* FrameQueue::PeekWritable()
{
    // Wait until we have space to put a new frame.
    SDL_LockMutex(mMutex);
    while(mSize >= mMaxSize && !mPacketQueue->Aborted())
    {
        SDL_CondWait(mSizeChangedCondition, mMutex);
    }
    SDL_UnlockMutex(mMutex);

    // Null if packet queue was aborted.
    if(mPacketQueue->Aborted())
    {
        return nullptr;
    }

    // Return frame at write index.
    return &mQueue[mWriteIndex];
}

Frame* FrameQueue::PeekReadable()
{
    // Wait until we have space to put a new frame.
    SDL_LockMutex(mMutex);
    while(mSize - mReadIndexOffset <= 0 && !mPacketQueue->Aborted())
    {
        SDL_CondWait(mSizeChangedCondition, mMutex);
    }
    SDL_UnlockMutex(mMutex);

    // Null if packet queue was aborted.
    if(mPacketQueue->Aborted())
    {
        return NULL;
    }

    return &mQueue[(mReadIndex + mReadIndexOffset) % mMaxSize];
}

void FrameQueue::Signal()
{
    SDL_LockMutex(mMutex);
    SDL_CondSignal(mSizeChangedCondition);
    SDL_UnlockMutex(mMutex);
}
