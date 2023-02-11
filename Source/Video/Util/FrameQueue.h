//
// FrameQueue.h
//
// Based very heavily off of the "FrameQueue" struct in ffmpeg's ffplay example.
//
// A queue of "frames" from ffmpeg.
// 
// A frame contains decoded (decompressed) video or audio data.
// Frames are obtained by decoding packets using the appropriate codec.
//
#pragma once

extern "C"
{
    #include <libavformat/avformat.h> // For AVFrame, AVSubtitle, AVRational
}
#include <SDL.h>

struct PacketQueue;

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

struct Frame
{
    // Audio/Video streams: decoded and ready to present AVFrame.
    // Subtitle streams: decoded and ready to present AVSubtitle object.
    AVFrame* frame = nullptr;
    AVSubtitle sub;
    
    // The serial for this frame - used to detect stale frames that can be tossed.
    int serial = 0;
    
    // Presentation timestamp for the frame.
    double pts = 0.0;
    
    // Estimated duration of the frame.
    double duration = 0.0;
    
    // Byte position of the frame in the input file.
    int64_t pos = 0L;
    
    // VIDEO PLAYBACK
    // Width and height of the video.
    // Cached here b/c value is retrieved from different spots for video/subtitles.
    int width = 0;
    int height = 0;
    
    // Aspect ratio of the frame.
    AVRational aspectRatio;
    
    // True if the frame has been uploaded to GPU.
    bool uploaded = false;
    
    // True if the image needs to be flipped vertically to display correctly.
    bool flipVertical = false;
    
    Frame();
    ~Frame();
    
    void Unref();
};

struct FrameQueue
{
    int Init(PacketQueue* pktq, int max_size, bool keep_last);
    void Destroy();
    
    // Enqueue or dequeue an element in the queue. It may seem unusual that Enqueue takes no argument.
    // You should use "PeekWritable" to get a frame, fill it, and then call Enqueue to indicate it is ready.
    void Enqueue();
    void Dequeue();
    
    Frame* Peek() { return &mQueue[(mReadIndex + mReadIndexOffset) % mMaxSize]; }
    Frame* PeekNext() { return &mQueue[(mReadIndex + mReadIndexOffset + 1) % mMaxSize]; }
    Frame* PeekLast() { return &mQueue[mReadIndex]; }
    Frame* PeekWritable();
    Frame* PeekReadable();
    
    int GetUndisplayedCount() { return mSize - mReadIndexOffset; }
    
    // If size is > 0, we have a frame.
    // Even if size is zero, if "keep last" was true (causing mReadIndexOffset to be 1), we always have a readable frame.
    bool HasReadableFrame() const { return mSize > 0 || mReadIndexOffset != 0; }
    
    void Signal();
    void LockMutex() { SDL_LockMutex(mMutex); }
    void UnlockMutex() { SDL_UnlockMutex(mMutex); }
    
private:
    // Holds a number of frames.
    Frame mQueue[FRAME_QUEUE_SIZE];
    
    // Current size and max size of the queue.
    // Often "max size" will equal FRAME_QUEUE_SIZE, but it's possible to force a smaller max size too.
    int mSize = 0;
    int mMaxSize = 0;
    
    // Decoder will first write frames to the queue, and then the playback system will read frames.
    // You'd expect the write index to always be somewhat ahead of the read index.
    int mReadIndex = 0;
    int mWriteIndex = 0;
    
    // If true, always keep at least one readable frame present. Used for video and subtitles.
    // This sets read index offset to 0/1 depending - effect is to skip removing first time "Next" is called.
    bool mKeepLastFrame = false;
    int mReadIndexOffset = 0;
    
    // PeekReadable/PeekWritable may block if there's no readable frame or no space to write.
    // This condition is signaled if queue size changes so they can unblock.
    SDL_cond* mSizeChangedCondition = nullptr;
    SDL_mutex* mMutex = nullptr;
    
    // The packet queue for this data stream.
    // Mainly needed to detect if an abort occurs.
    PacketQueue* mPacketQueue = nullptr;
};
