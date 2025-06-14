#include "VideoPlayer.h"

#include "Actor.h"
#include "AssetManager.h"
#include "InputManager.h"
#include "ReportManager.h"
#include "Texture.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UIUtil.h"
#include "VideoState.h"
#include "Window.h"

VideoPlayer gVideoPlayer;

VideoPlayer::VideoPlayer() :
    mLayer("MovieLayer")
{
    // Movie layer should override all game audio.
    mLayer.OverrideAudioState(true);
}

VideoPlayer::~VideoPlayer()
{
    Stop();
}

void VideoPlayer::Initialize()
{
    // Create canvas actor that sticks around forever.
    mVideoCanvasActor = new Actor("VideoPlayer", TransformType::RectTransform);
    mVideoCanvasActor->SetIsDestroyOnLoad(false);
    UI::AddCanvas(mVideoCanvasActor, 15, Color32::Black);

    // When a video plays, a background image either tints or completely blocks whatever's behind the video.
    // This is created along with the Canvas in AddCanvas - grab a reference to it here.
    mVideoBackgroundImage = mVideoCanvasActor->GetComponent<UIImage>();

    // Create black background image, used for letterbox effect.
    mVideoLetterbox = UI::CreateWidgetActor<UIImage>("Letterbox", mVideoCanvasActor);
    mVideoLetterbox->SetColor(Color32::Black);

    // Create video image, which shows actual video playback.
    mVideoImage = UI::CreateWidgetActor<UIImage>("Video", mVideoCanvasActor);
    mVideoImage->SetTexture(&Texture::Black);

    // Disable video UI until a movie is played.
    mVideoCanvasActor->SetActive(false);
}

void VideoPlayer::Update()
{
    // Update video playback and video texture.
    if(mVideo != nullptr)
    {
        // Update video state (this fills video texture with new data as video plays back).
        mVideo->Update();

        // Apply video texture.
        Texture* videoTexture = mVideo->GetVideoTexture();
        if(mHasTransparentColor && videoTexture != nullptr)
        {
            videoTexture->SetTransparentColor(mTransparentColor);
        }
        if(mOverrideVideoImage != nullptr)
        {
            mOverrideVideoImage->SetTexture(videoTexture);
        }
        else
        {
            mVideoImage->SetTexture(videoTexture);
        }

        // A lot of this logic only applies to the "theater viewing experience".
        // When using an override image (probably on an in-game UI texture), a lot of this doesn't apply.
        if(mOverrideVideoImage == nullptr)
        {
            // Set position of video image and background.
            mVideoLetterbox->GetRectTransform()->SetAnchoredPosition(mVideoPosition);
            mVideoImage->GetRectTransform()->SetAnchoredPosition(mVideoPosition);

            // Determine video size.
            Vector2 videoSize;
            switch(mVideoSizeMode)
            {
            case SizeMode::Native:
                if(videoTexture != nullptr)
                {
                    videoSize = Vector2(videoTexture->GetWidth(), videoTexture->GetHeight());
                }
                break;

            case SizeMode::Fullscreen:
                videoSize = Window::GetSize();
                break;

            case SizeMode::Custom:
                videoSize = mCustomVideoSize;
                break;
            }

            // For fullscreen videos, don't allow upscaling more than 2x original size.
            // This is a restriction used by the original engine.
            // I think it's meant to stop low-res videos from looking too pixelated on larger displays.
            if(mVideoSizeMode == SizeMode::Fullscreen && videoTexture != nullptr)
            {
                unsigned int maxVideoWidth = videoTexture->GetWidth() * 2;
                unsigned int maxVideoHeight = videoTexture->GetHeight() * 2;

                if(videoSize.x > maxVideoWidth)
                {
                    videoSize.x = maxVideoWidth;
                }
                if(videoSize.y > maxVideoHeight)
                {
                    videoSize.y = maxVideoHeight;
                }
            }

            // Always set the letterbox to the "expected" size for video rendering.
            // If letterboxing occurs, the video itself will be a different size than the letterbox image behind it.
            mVideoLetterbox->GetRectTransform()->SetSizeDelta(videoSize);

            // If letterboxing is enabled, recalculate video size so to not stretch the video.
            // If not letterboxing, and video size doesn't match the video's aspect ratio, stretching or warping may occur.
            if(mLetterbox && videoTexture != nullptr)
            {
                // Letterbox: fit the area, but preserve aspect ratio.
                mVideoImage->ResizeToFitPreserveAspect(videoSize);
            }
            else
            {
                // No letterbox, just use video size as-is.
                mVideoImage->GetRectTransform()->SetSizeDelta(videoSize);
            }

            // Pressing escape skips the video.
            if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_ESCAPE))
            {
                Stop();
            }
        }

        // Check for video end - call Stop if so to clean up video and call callback.
        if(mVideo != nullptr && mVideo->IsStopped())
        {
            Stop();
        }
    }

    // Only show video image when a video is playing.
    // Update UI after updating video playback in case video stops or ends prematurely.
    mVideoCanvasActor->SetActive(mOverrideVideoImage == nullptr && mVideo != nullptr);
}

void VideoPlayer::Play(const std::string& name)
{
    Play(name, false, true, nullptr);
}

void VideoPlayer::Play(const std::string& name, bool fullscreen, bool autoclose, std::function<void()> stopCallback)
{
    // Stop any video that is already playing first.
    Stop();

    // Push video layer.
    gLayerManager.PushLayer(&mLayer);

    // Log movie play.
    gReportManager.Log("Generic", "PlayMovie: trying to play " + name);

    // Save callback.
    mStopCallback = stopCallback;

    // If fullscreen, use window size as video size.
    // If not fullscreen, use size from video file.
    mVideoSizeMode = fullscreen ? SizeMode::Fullscreen : SizeMode::Native;

    // This video will use the built-in video image, so null out any override.
    mOverrideVideoImage = nullptr;

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
        tintColor.SetA(kNonFullscreenAlpha);
        mVideoBackgroundImage->SetColor(tintColor);
    }

    // Save autoclose flag.
    mAutoclose = autoclose;

    // The name passed is just a filename (e.g. "intro.bik"). Need to convert that into a full path.
    // Names can also be passed with or without extension, so we have to try to resolve any ambiguous name.
    std::string videoPath = gAssetManager.GetAssetPath(name, { "bik", "avi" });

    // Create new video.
    mVideo = new VideoState(videoPath.c_str());

    // On create, video begins playing immediately, assuming no error occurs.
    // If stopped, something happened, and video will not play.
    if(mVideo->IsStopped())
    {
        gReportManager.Log("Error", "No movie specified for the movie layer.");
        Stop();
        return;
    }

    // If we got here, movie seems to be playing ok!
    // Lock the mouse so it isn't visible and doesn't change position.
    gInputManager.LockMouse();

    // Show the video canvas and set the image black to start.
    // Doing this right away avoids a potential one-frame rendering the 3D scene before the movie starts.
    mVideoCanvasActor->SetActive(true);
    mVideoImage->SetTexture(&Texture::Black);
}

void VideoPlayer::Play(const std::string& name, Color32* transparentColor, UIImage* image, const std::function<void()>& callback)
{
    // Stop any video that is already playing first.
    Stop();

    // Store whether we have a transparent color.
    mHasTransparentColor = false;
    if(transparentColor != nullptr)
    {
        mHasTransparentColor = true;
        mTransparentColor = *transparentColor;
    }

    // Save override video image.
    mOverrideVideoImage = image;

    // Save stop callback.
    mStopCallback = callback;

    // The name passed is just a filename (e.g. "intro.bik"). Need to convert that into a full path.
    // Names can also be passed with or without extension, so we have to try to resolve any ambiguous name.
    std::string videoPath = gAssetManager.GetAssetPath(name, { "bik", "avi" });

    // Create new video.
    mVideo = new VideoState(videoPath.c_str());

    // On create, video begins playing immediately, assuming no error occurs.
    // If stopped, something happened, and video will not play.
    if(mVideo->IsStopped())
    {
        gReportManager.Log("Error", "No movie specified for the movie layer.");
        Stop();
        return;
    }
}

void VideoPlayer::Stop()
{
    // Pop video layer.
    if(mVideo != nullptr)
    {
        if(gLayerManager.IsTopLayer(&mLayer))
        {
            gLayerManager.PopLayer();
        }

        // Delete video to cleanup resources.
        delete mVideo;
        mVideo = nullptr;

        // Unlock mouse on movie end. TODO: Maybe do this in the video layer?
        gInputManager.UnlockMouse();
    }

    // Fire stop callback.
    if(mStopCallback != nullptr)
    {
        auto callback = mStopCallback;
        mStopCallback = nullptr;
        callback();
    }
}

