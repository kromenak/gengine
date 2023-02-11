//
// Clark Kromenaker
//
// Encapsulates playback of audio data through SDL audio subsystem.
//
#pragma once
#include <SDL.h>
extern "C"
{
    #include <libavformat/avformat.h>
}

struct SwrContext;
struct VideoState;

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB   20

struct AudioParams
{
    int freq;
    int channels;
    int64_t channel_layout;
    enum AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
};

class AudioPlaybackSDL
{
public:
    AudioPlaybackSDL();
    ~AudioPlaybackSDL();
    
    int Open(VideoState* is, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate);
    void Close();
    
private:
    // Handle for audio device opened via SDL - used to pause/resume/stop/etc.
    SDL_AudioDeviceID mAudioDeviceId = 0;
    
    // Size of the audio device buffer (cached when opening the audio device).
    int mAudioDeviceBufferSize = 0;
    
    // Format info for audio source (from frames) and audio target (playback device).
    // If these don't match, audio data must be resampled before being sent to output.
    AudioParams mAudioInParams;
    AudioParams mAudioOutParams;
    
    // Volume for audio playback. In SDL, the range is 0 to 128.
    // If muted, silence is emitted.
    int mVolume = SDL_MIX_MAXVOLUME;
    bool mMuted = false;
    
    // Points to the audio buffer containing audio data fit for playback.
    // When no resampling is needed, this points directly to data in the current Frame.
    // If resampling is needed, this points to the resample buffer.
    uint8_t* mAudioBuffer = nullptr;
    unsigned int mAudioBufferSize = 0; /* in bytes */
    
    // Current position within the audio buffer.
    // When this exceeds the size of the audio buffer, we must decode more data before playback can continue.
    int mAudioBufferIndex = 0; /* in bytes */
    
    // Contains info on how to resample from source to target format.
    // Only defined in resampling is required.
    SwrContext* mResampleContext = nullptr;
    
    // If data in the current frame can't be used directly for playback, it is resampled.
    // The resampled data is stored here for use during playback.
    uint8_t* mResampleBuffer = nullptr;
    unsigned int mResampleBufferSize = 0;
    
    // Contains clock (pts) data from most recently decoded Frame.
    // Used to set the audio clock for sync purposes.
    double audio_clock = 0.0;
    int audio_clock_serial = -1;
    
    // If audio is synced to video (uncommon), we must calculate whether audio is behind or ahead of video.
    // These values are used to track drift from master clock and apply corrections.
    double audio_diff_cum = 0.0; /* used for AV difference average computation */
    double audio_diff_avg_coef = 0.0;
    double audio_diff_threshold = 0.0;
    int audio_diff_avg_count = 0;
    
    static int64_t mAudioCallbackTime;
    static void AudioCallback(void* opaque, Uint8* stream, int len);
    
    int DecodeFrame(VideoState* is);
    int SyncAudio(VideoState* is, int nb_samples);
};
