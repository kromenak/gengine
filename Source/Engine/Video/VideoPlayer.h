//
// Clark Kromenaker
//
// A dedicated "video player" UI for watching videos that aren't integrated into other UI.
//
#pragma once
#include "Actor.h"

#include <functional>
#include <string>

#include "LayerManager.h"
#include "Vector2.h"

class UIImage;
class UIVideoImage;
struct VideoState;

class VideoPlayer : public Actor
{
public:
    VideoPlayer();

    void Show();
    void Hide();

    void Play(const std::string& videoName, bool fullscreen, bool autoclose, const std::function<void()>& callback);
    void Stop();

    bool IsPlaying() const;

    void AllowSkip(bool allow) { mAllowSkip = allow; }

protected:
    void OnUpdate(float deltaTime) override;

private:
    Layer mLayer;

    // A background image that takes up the full screen behind a video.
    // When playing a "fullscreen" video, this is totally black.
    // For non-fullscreen videos, it is slightly alpha'd so you can see below it.
    UIImage* mVideoBackgroundImage = nullptr;

    // If desired video playback size doesn't match the actual video, a solid black letterbox is applied.
    UIImage* mVideoLetterbox = nullptr;

    // Image that displays the actual video.
    UIVideoImage* mVideoImage = nullptr;

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

    // The custom video size when "Custom" video size mode is used.
    Vector2 mCustomVideoSize;

    // If true, video is letterboxed if aspect ratio of video doesn't match
    // aspect ratio of the on-screen area where the video is being displayed.
    bool mLetterbox = true;

    // If true, video will automatically close (i.e. hide itself) when end of playback is reached.
    bool mAutoclose = true;

    // Callback that is fired when video playback stops (either due to EOF or skip).
    std::function<void()> mStopCallback = nullptr;

    // If true, pressing ESC will skip the movie.
    bool mAllowSkip = true;

    void RefreshUI();
};