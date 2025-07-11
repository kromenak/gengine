#include "PacketQueue.h"

int PacketQueue::Init()
{
    // Clear all fields.
    memset(this, 0, sizeof(PacketQueue));

    // Create mutex or fail.
    mMutex = SDL_CreateMutex();
    if(!mMutex)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }

    // Create condition variable or fail.
    mNewPacketCondition = SDL_CreateCond();
    if(!mNewPacketCondition)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    return 0;
}

void PacketQueue::Destroy()
{
    Clear();
    SDL_DestroyMutex(mMutex);
    SDL_DestroyCond(mNewPacketCondition);
}

void PacketQueue::Start()
{
    mAborted = false;
    EnqueueFlush();
}

void PacketQueue::Abort()
{
    SDL_LockMutex(mMutex);
    mAborted = true;
    SDL_CondSignal(mNewPacketCondition);
    SDL_UnlockMutex(mMutex);
}

int PacketQueue::Enqueue(AVPacket* avPacket)
{
    SDL_LockMutex(mMutex);
    int ret = PutInternal(avPacket);
    SDL_UnlockMutex(mMutex);

    // Failed to put this packet in the queue.
    // Unref it right away because we are discarding it.
    if(ret < 0)
    {
        av_packet_unref(avPacket);
    }
    return ret;
}

int PacketQueue::EnqueueEof(int streamIndex)
{
    AVPacket avPacket;
    av_init_packet(&avPacket);
    avPacket.data = nullptr;
    avPacket.size = 0;
    avPacket.stream_index = streamIndex;
    return Enqueue(&avPacket);
}

int PacketQueue::EnqueueFlush()
{
    AVPacket avPacket;
    av_init_packet(&avPacket);
    avPacket.data = nullptr;
    avPacket.size = 0;
    avPacket.stream_index = -1;
    return Enqueue(&avPacket);
}

int PacketQueue::Dequeue(bool block, AVPacket* avPacket, int* avPacketSerial)
{
    SDL_LockMutex(mMutex);

    int ret = 0;
    while(true)
    {
        // Abort if requested.
        if(mAborted)
        {
            ret = -1;
            break;
        }

        // If there's a packet, remove it from the queue and set out variables.
        if(mFirstPacket != nullptr)
        {
            // Remove first packet from queue.
            Packet* firstPacket = mFirstPacket;
            mFirstPacket = firstPacket->next;
            if(mFirstPacket == nullptr)
            {
                mLastPacket = nullptr;
            }

            // Queue packet count, size, duration all decrease.
            mPacketCount--;
            mSizeBytes -= firstPacket->pkt.size + sizeof(*firstPacket);
            mDuration -= firstPacket->pkt.duration;

            // Set out vars.
            *avPacket = firstPacket->pkt;
            if(avPacketSerial)
            {
                *avPacketSerial = firstPacket->serial;
            }

            delete firstPacket;
            ret = 1;
            break;
        }
        else if(!block)
        {
            // No packet, but don't want to block.
            ret = 0;
            break;
        }
        else
        {
            // No packet and want to block until a packet becomes available.
            SDL_CondWait(mNewPacketCondition, mMutex);
        }
    }

    SDL_UnlockMutex(mMutex);
    return ret;
}

void PacketQueue::Clear()
{
    SDL_LockMutex(mMutex);

    // Clean up all packets in queue.
    Packet* pkt = mFirstPacket;
    while(pkt != nullptr)
    {
        Packet* temp = pkt;
        pkt = temp->next;

        // Unref packet - we're discarding it.
        av_packet_unref(&temp->pkt);
        delete temp; // av_freep(&temp);
    }

    // Queue is now empty.
    mLastPacket = nullptr;
    mFirstPacket = nullptr;
    mPacketCount = 0;
    mSizeBytes = 0;
    mDuration = 0;

    SDL_UnlockMutex(mMutex);
}

int PacketQueue::PutInternal(AVPacket* avPacket)
{
    // Don't put anything if aborting.
    if(mAborted) { return -1; }

    // Create packet or fail.
    Packet* packet = new Packet(); //static_cast<Packet*>(av_malloc(sizeof(Packet)));
    if(packet == nullptr) { return -1; }

    // Store AVPacket inside of packet.
    packet->pkt = *avPacket;

    // If it is the flush packet, increment serial.
    // The flush packet indicates that a skip/discontinuity has occurred in playback.
    // So, following packets are from a different segment than previous packets.
    if(avPacket->data == nullptr && avPacket->stream_index < 0)
    {
        serial++;
    }

    // Save packet's serial.
    packet->serial = serial;

    // Add to end of queue.
    if(mLastPacket == nullptr)
    {
        mFirstPacket = packet;
    }
    else
    {
        mLastPacket->next = packet;
    }
    mLastPacket = packet;

    // Increase count/size/duration.
    mPacketCount++;
    mSizeBytes += packet->pkt.size + sizeof(*packet);
    mDuration += packet->pkt.duration;
    /* XXX: should duplicate packet data in DV case */

    // If anybody's waiting on the "more packets" condition, let them know we have more packets.
    SDL_CondSignal(mNewPacketCondition);
    return 0;
}
