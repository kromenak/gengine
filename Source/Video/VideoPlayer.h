//
// VideoPlayer.h
//
// Clark Kromenaker
//
// Manager class for video playback.
//
#pragma once
#include <string>

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
    void Stop();
    
private:
    // The video being played; null if none.
    VideoState* mVideo = nullptr;
    
    // UI components used to show video.
    // Background is used for letterboxing effect, when desired.
    UIImage* mVideoBackground = nullptr;
    UIImage* mVideoImage = nullptr;
    
    // Position to show the video on-screen. Zero is center of screen.
    // Size of video on-screen. Fullscreen by default.
    Vector2 mVideoPosition;
    Vector2 mVideoSize;
    
    // If true, video is letterboxed if aspect ratio of video doesn't match
    // aspect ratio of the on-screen area where the video is being displayed.
    bool mLetterbox = false;
};
