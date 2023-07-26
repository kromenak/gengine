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

class Actor;
class UIImage;
struct VideoState;

class VideoPlayer
{
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
    
    // Root canvas for all video UI components.
    Actor* mVideoCanvasActor = nullptr;
    
    // A background image that takes up the full screen behind a video.
    // When playing a "fullscreen" video, this is totally black.
    // For non-fullscreen videos, it is slightly alpha'd so you can see below it.
    UIImage* mVideoBackgroundImage = nullptr;
    
    // If desired video playback size doesn't match the actual video, a solid black letterbox is applied.
    UIImage* mVideoLetterbox = nullptr;
    
    // Image that displays the actual video texture.
    UIImage* mVideoImage = nullptr;
    
    // Position to show the video on-screen. Zero is center of screen.
    Vector2 mVideoPosition;
    
    // Size of video on-screen. Fullscreen by default.
    enum class SizeMode
    {
        Native,         // Video plays at its native size.
        Fullscreen,     // Video plays in fullscreen (but with a maximum of 2x upscale).
        Custom          // Video plays at a completely custom size.
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

extern VideoPlayer gVideoPlayer;