//
// VideoPlayer.cpp
//
// Clark Kromenaker
//
#include "VideoPlayer.h"

#include "Actor.h"
#include "Services.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "VideoState.h"

TYPE_DEF_BASE(VideoPlayer);

VideoPlayer::VideoPlayer() :
    mLayer("MovieLayer")
{
    
}

VideoPlayer::~VideoPlayer()
{
    Stop();
}

void VideoPlayer::Initialize()
{
    // Create canvas actor.
    Actor* actor = new Actor(Actor::TransformType::RectTransform);
    actor->SetIsDestroyOnLoad(false);
    UICanvas* canvas = actor->AddComponent<UICanvas>();
    
    // Create black background image, used for letterbox effect.
    Actor* videoBackgroundActor = new Actor(Actor::TransformType::RectTransform);
    videoBackgroundActor->GetTransform()->SetParent(actor->GetTransform());
    mVideoBackground = videoBackgroundActor->AddComponent<UIImage>();
    mVideoBackground->SetColor(Color32::Black);
    canvas->AddWidget(mVideoBackground);
    
    // Create video image, which shows actual video playback.
    Actor* videoActor = new Actor(Actor::TransformType::RectTransform);
    videoActor->GetTransform()->SetParent(actor->GetTransform());
    mVideoImage = videoActor->AddComponent<UIImage>();
    canvas->AddWidget(mVideoImage);
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
        mVideoBackground->GetRectTransform()->SetAnchoredPosition(mVideoPosition);
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
            videoSize = Services::GetRenderer()->GetWindowSize();
            break;
            
        case SizeMode::Custom:
            videoSize = mCustomVideoSize;
            break;
        }
        mVideoBackground->GetRectTransform()->SetSizeDelta(videoSize);
        
        // If letterboxing, resize video image so as to not stretch the video.
        // The background will act as a "letterbox" if needed.
        if(mLetterbox && videoTexture != nullptr)
        {
            float videoWidth = videoTexture->GetWidth();
            float videoHeight = videoTexture->GetHeight();
            
            // Start by filling the width of the area.
            float widthRatio = videoSize.x / videoWidth;
            float newWidth = videoWidth * widthRatio;
            float newHeight = videoHeight * widthRatio;
            
            // If height is too large still for display area, scale again.
            if(newHeight > videoSize.y)
            {
                float heightRatio = videoSize.y / newHeight;
                newWidth *= heightRatio;
                newHeight *= heightRatio;
            }
            mVideoImage->GetRectTransform()->SetSizeDelta(Vector2(newWidth, newHeight));
        }
        else
        {
            // Video image just uses specified size, even if it doesn't match video's aspect ratio.
            // This means the video might be stretched and not correct aspect ratio.
            mVideoImage->GetRectTransform()->SetSizeDelta(videoSize);
        }
        
        // Check for video end - call Stop if so to clean up video and call callback.
        if(mVideo->IsStopped())
        {
            Stop();
        }
        
        // Pressing escape skips the video.
        if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_ESCAPE))
        {
            Stop();
        }
    }
    
    // Only show video image when a video is playing.
    // Update UI after updating video playback in case video stops or ends prematurely.
    mVideoImage->SetEnabled(mVideo != nullptr);
    mVideoBackground->SetEnabled(mVideo != nullptr);
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
    }
    
    // Delete video to cleanup resources.
    delete mVideo;
    mVideo = nullptr;
    
    // Unlock mouse on movie end.
    Services::GetInput()->UnlockMouse();
    
    // Fire stop callback.
    if(mStopCallback != nullptr)
    {
        mStopCallback();
        mStopCallback = nullptr;
    }
}

