//
// PacketQueue.h
//
// Based very heavily off of the "FrameQueue" struct in ffmpeg's ffplay example.
//
// A queue of "packets" from ffmpeg.
//
// Packets contain compressed data obtained after demuxing (separating audio/video/etc into separate streams).
// A packet contains compressed video or audio data, but it has yet to be decoded.
//
#pragma once

extern "C"
{
    #include <libavformat/avformat.h> // For AVPacket
}
#include <SDL.h>

struct PacketQueue
{
    // Incremented each time a flush packet is encountered (currently only Start and Seek).
    // Indicates what "sequence" or "section" the packet is part of. Seeking creates a discontinuity.
    int serial = 0;
    
    int Init();
    void Destroy();
    
    void Start();
    void Abort();
    bool Aborted() const { return mAborted; }
    
    // Return 0 on success, -1 on failure.
    int Enqueue(AVPacket* avPacket);
    int EnqueueEof(int streamIndex);
    int EnqueueFlush();
    
    // Return < 0 if aborted, 0 if no packet and > 0 if packet.
    int Dequeue(bool block, AVPacket* avPacket, int* serial);
    void Clear();
    
    int GetPacketCount() const { return mPacketCount; }
    int GetByteSize() const { return mSizeBytes; }
    int64_t GetDuration() const { return mDuration; }
    
    static bool IsEofPacket(AVPacket* avPacket) { return avPacket->data == nullptr && avPacket->stream_index >= 0; }
    static bool IsFlushPacket(AVPacket* avPacket) { return avPacket->data == nullptr && avPacket->stream_index < 0; }
    
private:
    // Helper to form a linked list of packets.
    struct Packet
    {
        AVPacket pkt;
        int serial = 0;
        Packet* next = nullptr;
    };
    
    // Front and end of queue.
    Packet* mFirstPacket = nullptr;
    Packet* mLastPacket = nullptr;
    
    // Number of packets in the queue.
    int mPacketCount = 0;
    
    // Size (in bytes) of all data in this queue.
    // Includes each packet's footprint + the size of the AVPacket contained.
    int mSizeBytes = 0;
    
    // Each packet has a duration for how long it will show - this is the sum of all in queue.
    int64_t mDuration = 0L;
    
    // Mutex handles multithread data modification.
    // Condition allows "Get" to block (if queue is empty) until a new packet is inserted.
    SDL_cond* mNewPacketCondition = nullptr;
    SDL_mutex* mMutex = nullptr;
    
    // If true, queue is aborted.
    bool mAborted = true;
    
    int PutInternal(AVPacket* avPacket);
};
