#include "VideoPlayer.h"

#include "Actor.h"
#include "InputManager.h"
#include "ReportManager.h"
#include "Texture.h"
#include "UIImage.h"
#include "UIUtil.h"
#include "UIVideoImage.h"
#include "Window.h"

VideoPlayer::VideoPlayer() : Actor("Video Player", TransformType::RectTransform),
    mLayer("MovieLayer")
{
    // Movie layer should override all game audio.
    mLayer.OverrideAudioState(true);

    // Create canvas that covers the screen.
    UI::AddCanvas(this, 15, Color32::Black);

    // When a video plays, a background image either tints or completely blocks whatever's behind the video.
    // This is created along with the Canvas in AddCanvas - grab a reference to it here.
    mVideoBackgroundImage = GetComponent<UIImage>();

    // Create black background image, used for letterbox effect.
    mVideoLetterbox = UI::CreateWidgetActor<UIImage>("Letterbox", this);
    mVideoLetterbox->SetColor(Color32::Black);

    // Create video image, which shows actual video playback.
    mVideoImage = UI::CreateWidgetActor<UIVideoImage>("Video", this);
    mVideoImage->SetTexture(&Texture::Black);

    // Disable video UI until a movie is played.
    SetActive(false);
}

void VideoPlayer::Show()
{
    // Push video layer.
    if(!IsActive())
    {
        gLayerManager.PushLayer(&mLayer);
        SetActive(true);
    }

    // If we got here, movie seems to be playing ok!
    // Lock the mouse so it isn't visible and doesn't change position.
    gInputManager.LockMouse();
}

void VideoPlayer::Hide()
{
    // If we're hiding, there's no need to continue video playback.
    // If video is still playing (Hide called without Stop), stop it here.
    mVideoImage->Stop();

    // Pop layer and set inactive.
    if(IsActive())
    {
        gLayerManager.PopLayer(&mLayer);
        SetActive(false);
    }

    // Unlock mouse on movie end.
    gInputManager.UnlockMouse();
}

void VideoPlayer::Play(const std::string& videoName, bool fullscreen, bool autoclose, const std::function<void()>& callback)
{
    // Log movie play.
    gReportManager.Log("Generic", "PlayMovie: trying to play " + videoName);

    // Play the video or fail out.
    // Note that, if playback fails (maybe video file is missing), the callback is still executed.
    bool videoPlaying = mVideoImage->Play(videoName, [this, callback](){
        if(mAutoclose)
        {
            Hide();
        }
        if(callback != nullptr)
        {
            callback();
        }
    });
    if(!videoPlaying)
    {
        gReportManager.Log("Error", "No movie specified for the movie layer.");
        return;
    }

    // If fullscreen, use window size as video size.
    // If not fullscreen, use size from video file.
    mVideoSizeMode = fullscreen ? SizeMode::Fullscreen : SizeMode::Native;

    // If fullscreen, background is 100% opaque.
    // If not fullscreen, background is alpha'd a bit.
    if(fullscreen)
    {
        mVideoBackgroundImage->SetColor(Color32::Black);
    }
    else
    {
        const unsigned char kNonFullscreenAlpha = 160;
        Color32 tintColor = Color32::Black;
        tintColor.a = kNonFullscreenAlpha;
        mVideoBackgroundImage->SetColor(tintColor);
    }

    // Save autoclose flag.
    mAutoclose = autoclose;

    // Show the video player UI.
    Show();

    // Right after starting video playback, make sure UI is positioned/sized correctly.
    RefreshUI();
}

void VideoPlayer::Stop()
{
    // Stop the video (this will fire any callbacks).
    mVideoImage->Stop();

    // If video is set to autoclose, then hide the video player.
    if(mAutoclose)
    {
        Hide();
    }
}

bool VideoPlayer::IsPlaying() const
{
    return mVideoImage->IsPlaying();
}

void VideoPlayer::OnUpdate(float deltaTime)
{
    // Keep UI properly positioned/sized during video playback.
    if(mVideoImage->IsPlaying())
    {
        RefreshUI();
    }

    // Pressing escape stops the video.
    if(mAllowSkip && gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_ESCAPE))
    {
        Stop();
    }
}

void VideoPlayer::RefreshUI()
{
    // Set position of video image and background.
    mVideoLetterbox->GetRectTransform()->SetAnchoredPosition(mVideoPosition);
    mVideoImage->GetRectTransform()->SetAnchoredPosition(mVideoPosition);

    // Determine video size.
    Vector2 videoSize;
    switch(mVideoSizeMode)
    {
        case SizeMode::Native:
        {
            Texture* videoTexture = mVideoImage->GetTexture();
            if(videoTexture != nullptr)
            {
                videoSize = Vector2(videoTexture->GetWidth(), videoTexture->GetHeight());
            }
            break;
        }
        case SizeMode::Fullscreen:
        {
            videoSize = static_cast<RectTransform*>(GetTransform())->GetSizeDelta();

            // For fullscreen videos, don't allow upscaling more than 2x original size.
            // This is a restriction used by the original engine.
            // I think it's meant to stop low-res videos from looking too pixelated on larger displays.
            Texture* videoTexture = mVideoImage->GetTexture();
            if(videoTexture != nullptr)
            {
                uint32_t maxVideoWidth = videoTexture->GetWidth() * 2;
                uint32_t maxVideoHeight = videoTexture->GetHeight() * 2;
                if(videoSize.x > maxVideoWidth)
                {
                    videoSize.x = maxVideoWidth;
                }
                if(videoSize.y > maxVideoHeight)
                {
                    videoSize.y = maxVideoHeight;
                }
            }
            break;
        }
        case SizeMode::Custom:
        {
            videoSize = mCustomVideoSize;
            break;
        }
    }

    // Always set the letterbox to the "expected" size for video rendering.
    // If letterboxing occurs, the video itself will be a different size than the letterbox image behind it.
    mVideoLetterbox->GetRectTransform()->SetSizeDelta(videoSize);

    // If letterboxing is enabled, recalculate video size so to not stretch the video.
    // If not letterboxing, and video size doesn't match the video's aspect ratio, stretching or warping may occur.
    if(mLetterbox)
    {
        // Letterbox: fit the area, but preserve aspect ratio.
        mVideoImage->ResizeToFitPreserveAspect(videoSize);
    }
    else
    {
        // No letterbox, just use video size as-is.
        mVideoImage->GetRectTransform()->SetSizeDelta(videoSize);
    }
}

