#include "AudioPlaybackSDL.h"

extern "C"
{
    #include <libavutil/time.h>
    #include <libavutil/samplefmt.h>
    #include <libswresample/swresample.h>
}

#include "VideoState.h"

/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512

/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

int64_t AudioPlaybackSDL::mAudioCallbackTime = 0L;

AudioPlaybackSDL::AudioPlaybackSDL()
{
    // Make sure SDL audio subsystem is initialized.
    // Note that it's OK to call this even if it has been previously initialized elsewhere.
    // This just increments a ref count (so we must call QuitSubSystem in destructor).
    SDL_InitSubSystem(SDL_INIT_AUDIO);
}

AudioPlaybackSDL::~AudioPlaybackSDL()
{
    // As mentioned in constructor, quit audio subsystem here.
    // If other parts of game are using SDL audio, the ref count system ensures that will keep working.
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

int AudioPlaybackSDL::Open(VideoState* is, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate)
{
    // Possible for audio channels argument to be an environment variable.
    // If so, override passed in values.
    const char* env = SDL_getenv("SDL_AUDIO_CHANNELS");
    if(env != nullptr)
    {
        wanted_nb_channels = atoi(env);
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
    }

    // If desired channel layout is not set (or doesn't make sense with desired number of channels), fallback on a default.
    if(wanted_channel_layout == 0 || wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout))
    {
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
        wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }
    wanted_nb_channels = av_get_channel_layout_nb_channels(wanted_channel_layout);
    
    // Populate wanted specification.
    SDL_AudioSpec wanted_spec;
    wanted_spec.channels = wanted_nb_channels;
    wanted_spec.freq = wanted_sample_rate;
    if(wanted_spec.freq <= 0 || wanted_spec.channels <= 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Invalid sample rate or channel count!\n");
        return -1;
    }
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.silence = 0;
    wanted_spec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(wanted_spec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
    wanted_spec.callback = AudioCallback;
    wanted_spec.userdata = is;
    
    // Determine lowest acceptable sample rate given wanted frequency.
    static const int next_sample_rates[] = { 0, 44100, 48000, 96000, 192000 };
    int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;
    while(next_sample_rate_idx > 0 && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq)
    {
        next_sample_rate_idx--;
    }
    
    // Loop and try to create audio device until it succeeds or we run out of options.
    SDL_AudioDeviceID audio_dev = 0;
    SDL_AudioSpec spec;
    while(audio_dev == 0)
    {
        // Attempt to open audio device with desired spec.
        audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
        if(audio_dev != 0)
        {
            break;
        }
        
        // Couldn't open audio device with desired spec, so need to fall back on other options.
        av_log(NULL, AV_LOG_WARNING, "SDL_OpenAudio (%d channels, %d Hz): %s\n",
               wanted_spec.channels, wanted_spec.freq, SDL_GetError());
        
        // Try a different number of channels (using a complicated system of channels->index).
        static const int next_nb_channels[] = { 0, 0, 1, 6, 2, 6, 4, 6 };
        wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
        
        // If zero channels, go down one sample rate...until we run out of sample rates...and fail.
        if(wanted_spec.channels == 0)
        {
            wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
            wanted_spec.channels = wanted_nb_channels;
            if(wanted_spec.freq == 0)
            {
                av_log(NULL, AV_LOG_ERROR, "No more combinations to try, audio open failed\n");
                return -1;
            }
        }
        wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
    }
    
    // Got an opened audio device! Save the device ID.
    mAudioDeviceId = audio_dev;
    
    // Fail if audio format doesn't match what we want.
    if(spec.format != AUDIO_S16SYS)
    {
        av_log(NULL, AV_LOG_ERROR, "SDL advised audio format %d is not supported!\n", spec.format);
        return -1;
    }
    
    // The supported channel spec may not match our desired channel spec.
    // In that case, try to update channel layout or fail.
    if(spec.channels != wanted_spec.channels)
    {
        wanted_channel_layout = av_get_default_channel_layout(spec.channels);
        if(wanted_channel_layout == 0)
        {
            av_log(NULL, AV_LOG_ERROR, "SDL advised channel count %d is not supported!\n", spec.channels);
            return -1;
        }
    }
    
    // Populate audio output format data.
    mAudioOutParams.fmt = AV_SAMPLE_FMT_S16;
    mAudioOutParams.freq = spec.freq;
    mAudioOutParams.channel_layout = wanted_channel_layout;
    mAudioOutParams.channels =  spec.channels;
    
    mAudioOutParams.frame_size = av_samples_get_buffer_size(NULL, mAudioOutParams.channels, 1, mAudioOutParams.fmt, 1);
    mAudioOutParams.bytes_per_sec = av_samples_get_buffer_size(NULL, mAudioOutParams.channels, mAudioOutParams.freq, mAudioOutParams.fmt, 1);
    if(mAudioOutParams.bytes_per_sec <= 0 || mAudioOutParams.frame_size <= 0)
    {
        av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n");
        return -1;
    }
    
    // By default, set audio input format equal to output format.
    // When we actually get audio data, if this ends up being incorrect,
    // we'll repopulate the input format and establish a resampling context.
    mAudioInParams = mAudioOutParams;
    
    // Init buffer info.
    mAudioDeviceBufferSize = spec.size;
    mAudioBufferSize  = 0;
    mAudioBufferIndex = 0;

    // Init averaging filter (for syncing audio to video or external clock).
    audio_diff_avg_coef  = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
    audio_diff_avg_count = 0;
    
    /* since we do not have a precise anough audio FIFO fullness,
       we correct audio sync only if larger than this threshold */
    audio_diff_threshold = (double)(mAudioDeviceBufferSize) / mAudioOutParams.bytes_per_sec;
    
    // Make sure audio is not paused.
    SDL_PauseAudioDevice(mAudioDeviceId, 0);
    return 0;
}

void AudioPlaybackSDL::Close()
{
    swr_free(&mResampleContext);
    av_freep(&mResampleBuffer);
    
    mResampleBufferSize = 0;
    mAudioBuffer = nullptr;
    
    SDL_CloseAudioDevice(mAudioDeviceId);
}


/*static*/ void AudioPlaybackSDL::AudioCallback(void* opaque, Uint8* stream, int len)
{
    VideoState* is = static_cast<VideoState*>(opaque);
    AudioPlaybackSDL* ap = is->audioPlayback;
    
    // Save callback time.
    mAudioCallbackTime = av_gettime_relative();
    
    // Loop until the stream is completely filled.
    while(len > 0)
    {
        // No more data left in the audio buffer.
        // We must get more data from the frame queue (including potentially resampling).
        if(ap->mAudioBufferIndex >= ap->mAudioBufferSize)
        {
           int audio_size = ap->DecodeFrame(is);
           if(audio_size < 0)
           {
               /* if error, just output silence */
               ap->mAudioBuffer = nullptr;
               ap->mAudioBufferSize = SDL_AUDIO_MIN_BUFFER_SIZE / ap->mAudioOutParams.frame_size * ap->mAudioOutParams.frame_size;
           }
           else
           {
               ap->mAudioBufferSize = audio_size;
           }
           ap->mAudioBufferIndex = 0;
        }
        
        // Amount of data available for playback is size - index (capped at size of stream).
        int len1 = ap->mAudioBufferSize - ap->mAudioBufferIndex;
        if(len1 > len)
        {
            len1 = len;
        }
        
        // If muted or missing audio buffer, just fill stream with silence.
        if(ap->mMuted || ap->mAudioBuffer == nullptr)
        {
            memset(stream, 0, len1);
        }
        else if(!ap->mMuted && ap->mAudioBuffer != nullptr)
        {
            // If at max volume, a simply copy will suffice.
            // If need to reduce volume, call the mix function to do that.
            if(ap->mVolume == SDL_MIX_MAXVOLUME)
            {
                memcpy(stream, (uint8_t *)ap->mAudioBuffer + ap->mAudioBufferIndex, len1);
            }
            else
            {
                SDL_MixAudioFormat(stream, (uint8_t *)ap->mAudioBuffer + ap->mAudioBufferIndex, AUDIO_S16SYS, len1, ap->mVolume);
            }
        }
        
        // Reduce length and increase stream pointer.
        // If len is still >0 (more stream to populate, we loop back around).
        len -= len1;
        stream += len1;
        
        // Increment audio buffer index as well.
        // If index surpasses available data in buffer, we need to get more data from frame queue before we can play back anymore.
        ap->mAudioBufferIndex += len1;
    }
    
    // Update audio clock so others can sync to us.
    if(!isnan(ap->audio_clock))
    {
        int audio_write_buf_size = ap->mAudioBufferSize - ap->mAudioBufferIndex;
        double pts = ap->audio_clock - (double)(2 * ap->mAudioDeviceBufferSize + audio_write_buf_size) / ap->mAudioOutParams.bytes_per_sec;
        is->audioClock.SetPtsAtTime(pts, ap->audio_clock_serial, mAudioCallbackTime / 1000000.0);
        is->externalClock.SyncTo(&is->audioClock);
    }
}

/**
 * Decode one audio frame and return its uncompressed size.
 *
 * The processed audio frame is decoded, converted if required, and
 * stored in is->audio_buf, with size in bytes given by the return
 * value.
 */
int AudioPlaybackSDL::DecodeFrame(VideoState* is)
{
    // If playback is paused, don't decode any audio.
    if(is->IsPaused())
    {
        return -1;
    }
    
    // Find a frame that's ready to play (and part of current serial).
    Frame* af = nullptr;
    do
    {
        // On Windows, I guess we need to sleep if we don't have any data (but why?).
        #if defined(_WIN32)
        while(is->audioFrames.GetUndisplayedCount() == 0)
        {
            if((av_gettime_relative() - mAudioCallbackTime) > (1000000LL * mAudioDeviceBufferSize / mAudioOutParams.bytes_per_sec / 2))
            {
                return -1;
            }
            av_usleep(1000);
        }
        #endif
        
        // Get playback frame or fail.
        af = is->audioFrames.PeekReadable();
        if(af == nullptr)
        {
            return -1;
        }
        is->audioFrames.Dequeue();
    } while(af->serial != is->audioPackets.serial);
    
    // Get buffer size required to hold data in this frame.
    int data_size = av_samples_get_buffer_size(nullptr, af->frame->channels,
                                               af->frame->nb_samples,
                                               (AVSampleFormat)af->frame->format, 1);

    // Determine channel layout for the audio data in this decoded frame.
    // Or, if not specificed, calculate it!
    int64_t dec_channel_layout =
        (af->frame->channel_layout != 0 && af->frame->channels == av_get_channel_layout_nb_channels(af->frame->channel_layout)) ?
        af->frame->channel_layout : av_get_default_channel_layout(af->frame->channels);
    
    // Determine wanted number of samples (which may differ from actual number of samples in the frame
    // if audio needs to sync to video or an external clock).
    int wanted_nb_samples = SyncAudio(is, af->frame->nb_samples);

    // If decoded audio in Frame does not match the format required by the playback device,
    // we need to resample the audio.
    if(af->frame->format        != mAudioInParams.fmt            ||
       dec_channel_layout       != mAudioInParams.channel_layout ||
       af->frame->sample_rate   != mAudioInParams.freq           ||
       (wanted_nb_samples       != af->frame->nb_samples && mResampleContext == nullptr))
    {
        // Free old resample context and create/init a new one.
        swr_free(&mResampleContext);
        mResampleContext = swr_alloc_set_opts(nullptr,
                                     mAudioOutParams.channel_layout, (AVSampleFormat)mAudioOutParams.fmt, mAudioOutParams.freq,
                                     dec_channel_layout,       (AVSampleFormat)af->frame->format, af->frame->sample_rate,
                                     0, nullptr);
        if(mResampleContext == nullptr || swr_init(mResampleContext) < 0)
        {
            av_log(NULL, AV_LOG_ERROR,
                   "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                    af->frame->sample_rate, av_get_sample_fmt_name((AVSampleFormat)af->frame->format), af->frame->channels,
                    mAudioOutParams.freq, av_get_sample_fmt_name(mAudioOutParams.fmt), mAudioOutParams.channels);
            swr_free(&mResampleContext);
            return -1;
        }
        
        // Update audio source format info (since it clearly didn't match what was in the frame).
        mAudioInParams.channel_layout = dec_channel_layout;
        mAudioInParams.channels = af->frame->channels;
        mAudioInParams.freq = af->frame->sample_rate;
        mAudioInParams.fmt = (AVSampleFormat)af->frame->format;
    }

    // If resample context is defined, it means the format of the decoded audio data (in the frame)
    // does not match the format required by the audio playback system.
    // So, we'll need to resample before playback can occur.
    int resampled_data_size = 0;
    if(mResampleContext != nullptr)
    {
        // Calculate size required for resample buffer.
        int out_count = static_cast<int>((int64_t)wanted_nb_samples * mAudioOutParams.freq / af->frame->sample_rate + 256);
        int out_size = av_samples_get_buffer_size(nullptr, mAudioOutParams.channels, out_count, mAudioOutParams.fmt, 0);
        if(out_size < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n");
            return -1;
        }
    
        // Set sample compensation if frame samples and desired samples don't match.
        if(wanted_nb_samples != af->frame->nb_samples)
        {
            if(swr_set_compensation(mResampleContext, (wanted_nb_samples - af->frame->nb_samples) * mAudioOutParams.freq / af->frame->sample_rate,
                                        wanted_nb_samples * mAudioOutParams.freq / af->frame->sample_rate) < 0)
            {
                av_log(NULL, AV_LOG_ERROR, "swr_set_compensation() failed\n");
                return -1;
            }
        }
        
        // Allocate large enough resample buffer.
        // Note that "fast_malloc" reuses existing buffer if already allocated and big enough.
        av_fast_malloc(&mResampleBuffer, &mResampleBufferSize, out_size);
        if(mResampleBuffer == nullptr)
        {
            return AVERROR(ENOMEM);
        }
        
        // Read IN from frame buffer and write resampled data OUT to resample buffer.
        const uint8_t** in = (const uint8_t**)af->frame->extended_data;
        uint8_t** out = &mResampleBuffer;
        
        // Convert frame data to resampled format, storing in the resample buffer.
        int createdCount = swr_convert(mResampleContext, out, out_count, in, af->frame->nb_samples);
        if(createdCount < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "swr_convert() failed\n");
            return -1;
        }
        
        // swr_convert returns number of samples created.
        // If equal to size of output buffer, this likely means the buffer was completely filled and some data was missed.
        if(createdCount == out_count)
        {
            av_log(NULL, AV_LOG_WARNING, "audio buffer is probably too small\n");
            if(swr_init(mResampleContext) < 0)
            {
                swr_free(&mResampleContext);
            }
        }
        
        // Use resample buffer as the audio buffer.
        mAudioBuffer = mResampleBuffer;
        resampled_data_size = createdCount * mAudioOutParams.channels * av_get_bytes_per_sample(mAudioOutParams.fmt);
    }
    else
    {
        // No need to resample - use frame data directly.
        mAudioBuffer = af->frame->data[0];
        resampled_data_size = data_size;
    }

    /*
    #ifdef DEBUG
    double prevAudioClock = audio_clock;
    #endif
    */
    
    // Update the audio clock with latest frame pts.
    if(!isnan(af->pts))
    {
        audio_clock = af->pts + (double)af->frame->nb_samples / af->frame->sample_rate;
    }
    else
    {
        audio_clock = NAN;
    }
    audio_clock_serial = af->serial;

    /*
    #ifdef DEBUG
    static double last_clock;
    printf("audio: delay=%0.3f clock=%0.3f clock0=%0.3f\n",
           audio_clock - last_clock,
           audio_clock, prevAudioClock);
    last_clock = audio_clock;
    #endif
    */
    return resampled_data_size;
}

int AudioPlaybackSDL::SyncAudio(VideoState* is, int nb_samples)
{
    // If audio is the sync master, no need to limit samples - use what we want!
    if(is->GetMasterSyncType() == AV_SYNC_AUDIO_MASTER)
    {
        return nb_samples;
    }
    
    // Not the master, so we need to sync to someone else (either video or external clock).
    // We may need to remove samples (to catch up) or add samples (to slow down).
    int wanted_nb_samples = nb_samples;
    
    // See if we are ahead (positive number) or behind (negative number).
    double diff = is->audioClock.GetTime() - is->GetMasterClock();
    
    // If within sync range, we'll try to correct.
    if(!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD)
    {
        // Add to cumulative diff.
        audio_diff_cum = diff + audio_diff_avg_coef * audio_diff_cum;
        
        // Must calculate a min number of diffs before average is good enough
        // to properly estimate changing samples for sync purposes.
        if(audio_diff_avg_count < AUDIO_DIFF_AVG_NB)
        {
            audio_diff_avg_count++;
        }
        else
        {
            /* estimate the A-V difference */
            double avg_diff = audio_diff_cum * (1.0 - audio_diff_avg_coef);
            if(fabs(avg_diff) >= audio_diff_threshold)
            {
                // Calculate number of samples wanted (smaller if ahead, bigger if behind).
                wanted_nb_samples = nb_samples + (int)(diff * mAudioInParams.freq);
                
                // Make sure corrected sample count is within min/max range to avoid too large jumps.
                int min_nb_samples = (nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100);
                int max_nb_samples = (nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100);
                wanted_nb_samples = av_clip(wanted_nb_samples, min_nb_samples, max_nb_samples);
            }
            av_log(NULL, AV_LOG_TRACE, "diff=%f adiff=%f sample_diff=%d apts=%0.3f %f\n",
                    diff, avg_diff, wanted_nb_samples - nb_samples,
                    audio_clock, audio_diff_threshold);
        }
    }
    else
    {
        // Diff is too large to consider syncing.
        // Just reset sync vars and hope for the best.
        audio_diff_avg_count = 0;
        audio_diff_cum       = 0;
    }

    return wanted_nb_samples;
}
