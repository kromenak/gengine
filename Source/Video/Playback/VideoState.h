//
// Clark Kromenaker
//
// A video that is being played.
// Data about playback state as well as controls for video playback.
//
#pragma once

extern "C"
{
    #include <libavformat/avformat.h>
}
#include <SDL.h>

#include "Decoder.h"
#include "FrameQueue.h"
#include "PacketQueue.h"
#include "PtsClock.h"

class AudioPlaybackSDL;
class VideoPlayback;
class Texture;

enum SyncType
{
    AV_SYNC_AUDIO_MASTER, /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

struct VideoState
{
    enum class State
    {
        Stopped,    // Video is not playing and will not continue playing.
        Playing,    // Video is currently playing.
        Paused      // Video playback is suspended, but can be resumed anytime.
    };
    
    // Video format data (streams that exist, codecs to use, etc).
    AVFormatContext* format = nullptr;
    
    // DATA PROCESSING
    // Data streams from the input file.
    AVStream* videoStream = nullptr;
    AVStream* audioStream = nullptr;
    AVStream* subtitleStream = nullptr;
    
    // ReadThread pulls AVPackets from the AVStreams and puts them in these queues.
    // Data in these queues is demuxed, but not yet decoded.
    PacketQueue videoPackets;
    PacketQueue audioPackets;
    PacketQueue subtitlePackets;
    
    // Decodes data from packet queues and puts decoded data into frame queues.
    Decoder videoDecoder;
    Decoder audioDecoder;
    Decoder subtitleDecoder;
    
    // Data in frame queue is decoded and ready to be presented.
    // Playback classes use this data to "present" (e.g. send to audio device, update texture, etc).
    FrameQueue videoFrames;
    FrameQueue audioFrames;
    FrameQueue subtitleFrames;
    
    // Audio and video clocks are updated as audio/video are presented. The external clock syncs to both.
    // Depending on synchronization desired (see SyncType enum), audio or video threads will attempt to correct for any desync.
    PtsClock videoClock;
    PtsClock audioClock;
    PtsClock externalClock;
    
    // AUDIO PLAYBACK
    AudioPlaybackSDL* audioPlayback = nullptr;
    
    // VIDEO PLAYBACK
    VideoPlayback* videoPlayback = nullptr;
    double frameTimer = 0.0;

    VideoState(const char* filename);
    ~VideoState();
    
    void Update();
    
    void TogglePause();
    void StepToNextFrame();
    
    bool IsPaused() const { return mState == State::Paused; }
    bool IsStopped() const { return mState == State::Stopped; }
    
    int GetMasterSyncType();
    double GetMasterClock();
    
    Texture* GetVideoTexture();
    
private:
    // Name of video file playing.
    char* mFilename = nullptr;
    
    // Thread used to read data from video file.
    // This thread reads data from the file into packet queues (performs demuxing).
    SDL_Thread* mReadThread = nullptr;
    
    // Whether to sync to audio, video, or an external clock.
    // Syncing to external *seems* to give much better results (at least for logo vids).
    // For long cutscenes with lip-syncing...we'll have to see.
    int mSyncType = AV_SYNC_EXTERNAL_CLOCK; //AV_SYNC_AUDIO_MASTER; 
    
    // If true, playback is aborted.
    bool mAborted = false;
    
    // Current playback state.
    State mState = State::Stopped;
    
    // Stream indexes for video, audio, and subtitles - determined by reading format data.
    int mVideoStreamIndex = -1;
    int mAudioStreamIndex = -1;
    int mSubtitleStreamIndex = -1;
    
    // If read thread believes all packet queues have enough data, it may rest for a bit, waiting 10ms OR until this condition is signaled.
    // On the flipside, decoders will signal this condition if they find that the packet queues are empty.
    SDL_cond* mContinueReadCondition = nullptr;
    
    // Seek functionality.
    bool seek_req = false;
    int seek_flags = 0;
    int64_t seek_pos = 0L;
    int64_t seek_rel = 0L;
    
    int OpenStream(int streamIndex);
    void CloseStream(int streamIndex);
    
    static int ReadThread(void* arg);
    void ReadThreadEnd(int result);
};
