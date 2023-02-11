//
// Based very heavily off of the "Decoder" struct in ffmpeg's ffplay example.
//
// A decoder uses a codec to decode packets from a packet queue.
// Decoded data are called "Frames" and are put into an appropriate FrameQueue.
//
// A decoder can potentially block indefinitely, so decoding occurs on a separate thread.
//
#pragma once

extern "C"
{
    #include <libavformat/avformat.h> // For AVRational
}
#include <SDL.h>

struct FrameQueue;
struct PacketQueue;
struct VideoState;

struct Decoder
{
    void Init(AVCodecContext* codecContext, PacketQueue* packetQueue, SDL_cond* emptyQueueCond);
    void Destroy();
    
    int Start(VideoState* is);
    void Abort(FrameQueue* frameQueue);
    
    int DecodeFrame(AVFrame* frame, AVSubtitle* sub);
    
    void SetFlushPts(int64_t flushPts, const AVRational& flushPtsTimeBase)
    {
        mFlushPts = flushPts;
        mFlushPtsTimeBase = flushPtsTimeBase;
    }
    
    bool ReachedEOF() const;
    AVCodecContext* GetCodecContext() const { return mCodecContext; }
    int GetSerial() const { return mSerial; }
    
private:
    // Super imporant - indicates what codec to use for decoding!
    AVCodecContext* mCodecContext = nullptr;
    
    // Queue of compressed packets that must be decoded.
    PacketQueue* mPacketQueue = nullptr;
    
    // Occasionally, a dequeued packet needs to be decoded more than once.
    // In that case, we just save the packet here until next decode loop so it can be used.
    AVPacket mPendingPacket { };
    bool mPacketPending = false;
    
    // A condition to signal if the packet queue becomes empty.
    // In practice, this always attempts to wake up the read thread.
    // If the packet queue is empty, the read thread needs to supply us with more packets!
    SDL_cond* mEmptyQueueCondition = nullptr;
    
    // Thread that this decoder uses for decoding.
    SDL_Thread* mDecoderThread = nullptr;
    
    // "next pts" and "next pts timebase" are reset to these values when a flush occurs.
    // Usually, default values are fine, but when stream can't seek, these are beginning of stream (aka restart the stream).
    int64_t mFlushPts = AV_NOPTS_VALUE;
    AVRational mFlushPtsTimeBase { 0, 0 };
    
    // For audio decoding, used to calculate pts for a decoded frame.
    int64_t mNextPts = AV_NOPTS_VALUE;
    AVRational mNextPtsTimeBase { 0, 0 };
    
    // The serial of last packet sent to decoder.
    int mSerial = -1;
    
    // Queue's serial when EOF was encountered.
    // Used to detect when decoder is out of stuff to do.
    int mEofSerial = 0;
};
