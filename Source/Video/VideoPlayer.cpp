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

VideoPlayer::VideoPlayer()
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
    // Only show video image when a video is playing.
    mVideoImage->SetEnabled(mVideo != nullptr);
    mVideoBackground->SetEnabled(mVideo != nullptr);
    
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
    }
}

void VideoPlayer::Play(const std::string& name)
{
    Play(name, false, true, nullptr);
}

void VideoPlayer::Play(const std::string& name, bool fullscreen, bool autoclose, std::function<void()> stopCallback)
{
    // Stop any video that is already playing first.
    Stop();
    
    // Log movie play.
    Services::GetReports()->Log("Generic", "PlayMovie: trying to play " + name);
    
    // The name passed is just a filename (e.g. "intro.bik"). Need to convert that into a full path.
    std::string videoPath = Services::GetAssets()->GetAssetPath(name);
    
    // If couldn't find video, don't play video!
    if(videoPath.empty())
    {
        if(stopCallback != nullptr)
        {
            stopCallback();
        }
        return;
    }
    
    // Create new video.
    mVideo = new VideoState(videoPath.c_str());
    
    // If fullscreen, use window size as video size.
    // If not fullscreen, use size from video file.
    mVideoSizeMode = fullscreen ? SizeMode::Fullscreen : SizeMode::Native;
    
    // Save autoclose flag.
    mAutoclose = autoclose;
    
    // Save callback.
    mStopCallback = stopCallback;
}

void VideoPlayer::Stop()
{
    // Delete video to cleanup resources.
    delete mVideo;
    mVideo = nullptr;
    
    // Fire stop callback.
    if(mStopCallback != nullptr)
    {
        mStopCallback();
        mStopCallback = nullptr;
    }
}

