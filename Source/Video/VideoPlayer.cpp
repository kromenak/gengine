#include "VideoPlayer.h"

#include "Actor.h"
#include "Services.h"
#include "Texture.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "VideoState.h"
#include "Window.h"

TYPE_DEF_BASE(VideoPlayer);

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
    mVideoCanvasActor = new Actor(TransformType::RectTransform);
    mVideoCanvasActor->SetIsDestroyOnLoad(false);
    mVideoCanvasActor->AddComponent<UICanvas>();
    
    // Size canvas rect so it always fills the entire screen.
    RectTransform* canvasRectTransform = mVideoCanvasActor->GetComponent<RectTransform>();
    canvasRectTransform->SetAnchorMin(Vector2::Zero);
    canvasRectTransform->SetAnchorMax(Vector2::One);
    canvasRectTransform->SetSizeDelta(Vector2::Zero);
    
    // When a video plays, a background image either tints or completely blocks whatever's behind the video.
    mVideoBackgroundImage = mVideoCanvasActor->AddComponent<UIImage>();
    
    // Create black background image, used for letterbox effect.
    Actor* videoBackgroundActor = new Actor(TransformType::RectTransform);
    videoBackgroundActor->GetTransform()->SetParent(mVideoCanvasActor->GetTransform());
    mVideoLetterbox = videoBackgroundActor->AddComponent<UIImage>();
    mVideoLetterbox->SetColor(Color32::Black);
    
    // Create video image, which shows actual video playback.
    Actor* videoActor = new Actor(TransformType::RectTransform);
    videoActor->GetTransform()->SetParent(mVideoCanvasActor->GetTransform());
    mVideoImage = videoActor->AddComponent<UIImage>();
    
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
        mVideoImage->SetTexture(videoTexture);
        
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

        // Check for video end - call Stop if so to clean up video and call callback.
        if(mVideo->IsStopped())
        {
            Stop();
        }
        
        // Pressing escape skips the video.
        if(Services::GetInput()->IsKeyLeadingEdge(SDL_SCANCODE_ESCAPE))
        {
            Stop();
        }
    }
    
    // Only show video image when a video is playing.
    // Update UI after updating video playback in case video stops or ends prematurely.
    mVideoCanvasActor->SetActive(mVideo != nullptr);
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
    Services::Get<LayerManager>()->PushLayer(&mLayer);
    
    // Log movie play.
    Services::GetReports()->Log("Generic", "PlayMovie: trying to play " + name);
    
    // Save callback.
    mStopCallback = stopCallback;
    
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
        tintColor.SetA(kNonFullscreenAlpha);
        mVideoBackgroundImage->SetColor(tintColor);
    }
    
    // Save autoclose flag.
    mAutoclose = autoclose;
    
    // The name passed is just a filename (e.g. "intro.bik"). Need to convert that into a full path.
    // Names can also be passed with or without extension, so we have to try to resolve any ambiguous name.
    std::string videoPath = Services::GetAssets()->GetAssetPath(name, { "bik", "avi" });
    
    // Create new video.
    mVideo = new VideoState(videoPath.c_str());
    
    // On create, video begins playing immediately, assuming no error occurs.
    // If stopped, something happened, and video will not play.
    if(mVideo->IsStopped())
    {
        Services::GetReports()->Log("Error", "No movie specified for the movie layer.");
        Stop();
        return;
    }
    
    // If we got here, movie seems to be playing ok!
    // Lock the mouse so it isn't visible and doesn't change position.
    Services::GetInput()->LockMouse();
    
    //TODO: Some video files have subtitles associated with them, but the subtitles are not part of the movie file.
    //TODO: Need to check for a YAK file of the same name and use that to display subtitles during movie playback.
}

void VideoPlayer::Stop()
{
    // Pop video layer.
    if(mVideo != nullptr)
    {
        Services::Get<LayerManager>()->PopLayer();
    
        // Delete video to cleanup resources.
        delete mVideo;
        mVideo = nullptr;

        // Unlock mouse on movie end. TODO: Maybe do this in the video layer?
        Services::GetInput()->UnlockMouse();
    }
    
    // Fire stop callback.
    if(mStopCallback != nullptr)
    {
        auto callback = mStopCallback;
        mStopCallback = nullptr;
        callback();
    }
}

