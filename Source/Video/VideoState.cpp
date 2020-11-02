//
// VideoState.cpp
//
// Clark Kromenaker
//
#include "VideoState.h"

extern "C"
{
    #include <libavutil/time.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}

#include "AudioPlaybackSDL.h"
#include "VideoPlaybackGL.h"

VideoState::VideoState(const char* filename) :
    mFilename(av_strdup(filename))
{
    // Initialize packet queues or fail.
    if(videoPackets.Init() < 0 ||
       audioPackets.Init() < 0 ||
       subtitlePackets.Init() < 0)
    {
        return;
    }
    
    // Initialize frame queues, or fail.
    if(videoFrames.Init(&videoPackets, VIDEO_PICTURE_QUEUE_SIZE, true) < 0 ||
       audioFrames.Init(&audioPackets, SAMPLE_QUEUE_SIZE, true) < 0 ||
       subtitleFrames.Init(&subtitlePackets, SUBPICTURE_QUEUE_SIZE, false) < 0)
    {
        return;
    }
    
    // Create continue read condition or fail.
    mContinueReadCondition = SDL_CreateCond();
    if(mContinueReadCondition == nullptr)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return;
    }
    
    // Init clocks.
    videoClock = PtsClock(&videoPackets.serial);
    audioClock = PtsClock(&audioPackets.serial);
    externalClock = PtsClock(nullptr);
    
    // Allocate format context or fail.
    format = avformat_alloc_context();
    if(format == nullptr)
    {
        av_log(NULL, AV_LOG_FATAL, "Could not allocate context.\n");
        return;
    }
    
    // Set interrupt callback - basically if IO is not possible, just abort.
    format->interrupt_callback.opaque = this;
    format->interrupt_callback.callback = [](void* arg) -> int {
        VideoState* is = static_cast<VideoState*>(arg);
        return is->mAborted;
    };
    
    // Open video file, or fail.
    int res = avformat_open_input(&format, filename, nullptr, nullptr);
    if(res < 0) {
        av_log(NULL, AV_LOG_FATAL, "Could not open video file.\n");
        return;
    }
    
    // Put global side data in first packets received from stream (and packets after a seek).
    //TODO: Why is this important/needed?
    av_format_inject_global_side_data(format);
    
    // Populate format context with stream info, or fail.
    res = avformat_find_stream_info(format, nullptr);
    if(res < 0)
    {
        av_log(NULL, AV_LOG_WARNING, "%s: could not find codec parameters\n", filename);
        return;
    }
    
    // Reset EOF flag on I/O context, so we can catch EOF later, I guess?
    if(format->pb)
    {
        format->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use avio_feof() to test for the end
    }
    
    // Output format info to log for debugging.
    av_dump_format(format, 0, filename, 0);
    
    // Find stream indexes for video/audio/subs.
    int videoIndex = av_find_best_stream(format, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    int audioIndex = av_find_best_stream(format, AVMEDIA_TYPE_AUDIO, -1, videoIndex, NULL, 0);
    int subtitlesIndex = av_find_best_stream(format, AVMEDIA_TYPE_SUBTITLE, -1, audioIndex, NULL, 0);
    
    // Open streams.
    if(videoIndex >= 0)
    {
        OpenStream(videoIndex);
    }
    if(audioIndex >= 0)
    {
        OpenStream(audioIndex);
    }
    if(subtitlesIndex >= 0)
    {
        OpenStream(subtitlesIndex);
    }
    
    // Could not open video or audio!?
    if(mVideoStreamIndex < 0 && mAudioStreamIndex < 0)
    {
        av_log(NULL, AV_LOG_FATAL, "Failed to open video/audio for '%s'\n", filename);
        return;
    }
    
    // Create read thread or fail.
    mReadThread = SDL_CreateThread(ReadThread, "read_thread", this);
    if(mReadThread == nullptr)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateThread(): %s\n", SDL_GetError());
    }
}

VideoState::~VideoState()
{
    // Tell video to abort and wait for read thread to exit.
    mAborted = true;
    SDL_WaitThread(mReadThread, nullptr);

    // Close each stream.
    if(mAudioStreamIndex >= 0)
    {
        CloseStream(mAudioStreamIndex);
    }
    if(mVideoStreamIndex >= 0)
    {
        CloseStream(mVideoStreamIndex);
    }
    if(mSubtitleStreamIndex >= 0)
    {
        CloseStream(mSubtitleStreamIndex);
    }

    // Close input file.
    avformat_close_input(&format);

    // Destroy packet queues.
    videoPackets.Destroy();
    audioPackets.Destroy();
    subtitlePackets.Destroy();

    // Destroy frame queues.
    videoFrames.Destroy();
    audioFrames.Destroy();
    subtitleFrames.Destroy();
    
    // Free condition variable.
    SDL_DestroyCond(mContinueReadCondition);
    
    // Free filename memory.
    av_free(mFilename);
}

void VideoState::Update()
{
    videoPlayback->Update(this);
}

void VideoState::TogglePause()
{
    if(mPaused)
    {
        frameTimer += videoClock.GetSecondsSinceLastUpdate();
        videoClock.SetPaused(false);
        videoClock.SetPtsToCurrentTime();
    }
    externalClock.SetPtsToCurrentTime();
    
    // Toggle pause.
    mPaused = !mPaused;
    
    // Propogate to all clocks.
    audioClock.SetPaused(mPaused);
    videoClock.SetPaused(mPaused);
    externalClock.SetPaused(mPaused);
    
    videoPlayback->SetStep(false);
}

void VideoState::StepToNextFrame()
{
    /* if the stream is paused unpause it, then step */
    if(mPaused)
    {
        TogglePause();
    }
    videoPlayback->SetStep(true);
}

int VideoState::GetMasterSyncType()
{
    if(mSyncType == AV_SYNC_VIDEO_MASTER)
    {
        if(videoStream)
            return AV_SYNC_VIDEO_MASTER;
        else
            return AV_SYNC_AUDIO_MASTER;
    }
    else if(mSyncType == AV_SYNC_AUDIO_MASTER)
    {
        if(audioStream)
            return AV_SYNC_AUDIO_MASTER;
        else
            return AV_SYNC_EXTERNAL_CLOCK;
    }
    else
    {
        return AV_SYNC_EXTERNAL_CLOCK;
    }
}

double VideoState::GetMasterClock()
{
    switch(GetMasterSyncType())
    {
    case AV_SYNC_VIDEO_MASTER:
        return videoClock.GetTime();
    case AV_SYNC_AUDIO_MASTER:
        return audioClock.GetTime();
    default:
        return externalClock.GetTime();
    }
}

Texture* VideoState::GetVideoTexture()
{
    return videoPlayback->GetVideoTexture();
}

int VideoState::OpenStream(int streamIndex)
{
    // Make sure stream index is in valid range.
    if(streamIndex < 0 || streamIndex >= format->nb_streams)
    {
        return -1;
    }
    
    // All streams should discard useless (e.g. 0-sized packets).
    format->streams[streamIndex]->discard = AVDISCARD_DEFAULT;

    // Allocate codec context or fail.
    AVCodecContext* avctx = avcodec_alloc_context3(nullptr);
    if(avctx == nullptr)
    {
        return AVERROR(ENOMEM);
    }
    
    // Populate codec context with codec parameters from the format context stream info.
    int ret = avcodec_parameters_to_context(avctx, format->streams[streamIndex]->codecpar);
    if(ret < 0)
    {
        avcodec_free_context(&avctx);
        return ret;
    }
    avctx->pkt_timebase = format->streams[streamIndex]->time_base;
    
    // We need a decoder in order to decode the stream data for playback.
    // Find the decoder from the codec ID we got from the stream info.
    AVCodec* codec = avcodec_find_decoder(avctx->codec_id);
    if(codec == nullptr)
    {
        av_log(NULL, AV_LOG_WARNING, "No decoder could be found for codec %s\n", avcodec_get_name(avctx->codec_id));
        ret = AVERROR(EINVAL);
        avcodec_free_context(&avctx);
        return ret;
    }
    avctx->codec_id = codec->id;

    // Init codec context to use provided codec.
    // Must be done before decoding can occur.
    ret = avcodec_open2(avctx, codec, nullptr);
    if(ret < 0)
    {
        avcodec_free_context(&avctx);
        return ret;
    }

    // Take some specific actions based on codec type (audio, video, etc).
    switch(avctx->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        audioPlayback = new AudioPlaybackSDL();
        
        // Open audio playback. Returns < 0 on error, audio buffer size on success.
        ret = audioPlayback->Open(this, avctx->channel_layout, avctx->channels, avctx->sample_rate);
        if(ret < 0)
        {
            avcodec_free_context(&avctx);
            return ret;
        }
        
        // Save stream info.
        mAudioStreamIndex = streamIndex;
        audioStream = format->streams[streamIndex];

        // Init audio decoder.
        audioDecoder.Init(avctx, &audioPackets, mContinueReadCondition);
        
        // For audio, if seek isn't allowed by this stream type, flushing (during start/seek) must always start at beginning of stream.
        // So, set the pts/timebase when flushing to be the start time.
        if((format->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) && !format->iformat->read_seek)
        {
            audioDecoder.SetFlushPts(audioStream->start_time, audioStream->time_base);
        }
        
        ret = audioDecoder.Start(this);
        break;
        
    case AVMEDIA_TYPE_VIDEO:
        videoPlayback = new VideoPlaybackGL();
        
        // Save stream info.
        mVideoStreamIndex = streamIndex;
        videoStream = format->streams[streamIndex];

        // Init video decoder.
        videoDecoder.Init(avctx, &videoPackets, mContinueReadCondition);
        ret = videoDecoder.Start(this);
        break;
        
    case AVMEDIA_TYPE_SUBTITLE:
        // Save stream info.
        mSubtitleStreamIndex = streamIndex;
        subtitleStream = format->streams[streamIndex];

        // Init subtitle decoder.
        subtitleDecoder.Init(avctx, &subtitlePackets, mContinueReadCondition);
        ret = subtitleDecoder.Start(this);
        break;
        
    default:
        av_log(NULL, AV_LOG_ERROR, "Unexpected codec type!");
        break;
    }
    return ret;
}

void VideoState::CloseStream(int streamIndex)
{
    // Make sure stream index is in valid range.
    if(streamIndex < 0 || streamIndex >= format->nb_streams)
    {
        return;
    }
    
    // Discard all packets (since the stream is now closed).
    format->streams[streamIndex]->discard = AVDISCARD_ALL;

    // Disable stream decoder and other resources.
    AVCodecParameters* codecpar = format->streams[streamIndex]->codecpar;
    switch(codecpar->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        audioDecoder.Abort(&audioFrames);
        audioDecoder.Destroy();
        audioPlayback->Close();
        delete audioPlayback;
        break;
    case AVMEDIA_TYPE_VIDEO:
        videoDecoder.Abort(&videoFrames);
        videoDecoder.Destroy();
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        subtitleDecoder.Abort(&subtitleFrames);
        subtitleDecoder.Destroy();
        break;
    default:
        av_log(NULL, AV_LOG_ERROR, "Unexpected codec type!");
        break;
    }

    // Clear stream pointer and index.
    switch(codecpar->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        audioStream = nullptr;
        mAudioStreamIndex = -1;
        break;
    case AVMEDIA_TYPE_VIDEO:
        videoStream = nullptr;
        mVideoStreamIndex = -1;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        subtitleStream = nullptr;
        mSubtitleStreamIndex = -1;
        break;
    default:
        av_log(NULL, AV_LOG_ERROR, "Unexpected codec type!");
        break;
    }
}
