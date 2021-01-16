//
// VideoPlayer.h
//
// Clark Kromenaker
//
// Manager class for video playback.
//
#pragma once
#include <functional>
#include <string>

#include "LayerManager.h"
#include "Type.h"
#include "Vector2.h"

class UIImage;
struct VideoState;

class VideoPlayer
{
    TYPE_DECL_BASE();
public:
    VideoPlayer();
    ~VideoPlayer();
    
    void Initialize();
    void Update();
    
    void Play(const std::string& name);
    void Play(const std::string& name, bool fullscreen, bool autoclose, std::function<void()> stopCallback);
    void Stop();
    
private:
    Layer mLayer;
    
    // The video being played; null if none.
    VideoState* mVideo = nullptr;
    
    // UI components used to show video.
    // Background is used for letterboxing effect, when desired.
    UIImage* mVideoBackground = nullptr;
    UIImage* mVideoImage = nullptr;
    
    // Position to show the video on-screen. Zero is center of screen.
    Vector2 mVideoPosition;
    
    // Size of video on-screen. Fullscreen by default.
    enum class SizeMode
    {
        Native,
        Fullscreen,
        Custom
    };
    SizeMode mVideoSizeMode = SizeMode::Native;
    Vector2 mCustomVideoSize;
    
    // If true, video is letterboxed if aspect ratio of video doesn't match
    // aspect ratio of the on-screen area where the video is being displayed.
    bool mLetterbox = true;
    
    // If true, video will automatically close (i.e. hide itself) when end of playback is reached.
    bool mAutoclose = true;
    
    // Callback that is fired when video playback stops (either due to EOF or skip).
    std::function<void()> mStopCallback = nullptr;
};
