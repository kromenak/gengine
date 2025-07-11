//
// Clark Kromenaker
//
// A UI image that displays a video.
//
#pragma once
#include "UIImage.h"

#include <functional>
#include <string>

class Texture;
struct VideoState;

class UIVideoImage : public UIImage
{
public:
    UIVideoImage(Actor* owner);
    ~UIVideoImage();

    bool Play(const std::string& videoName, const std::function<void()>& callback = nullptr);
    bool Play(const std::string& videoName, const Color32& transparentColor, const std::function<void()>& callback = nullptr);
    void Stop();

    bool IsPlaying() const;

protected:
    void OnUpdate(float deltaTime) override;

private:
    // The video being played.
    VideoState* mVideo = nullptr;

    // The texture the video is rendering to and which is being displayed by this image.
    Texture* mVideoTexture = nullptr;

    // A callback to fire when video playback ends (either naturally or prematurely).
    std::function<void()> mCallback = nullptr;

    void EndVideoPlayback();
};