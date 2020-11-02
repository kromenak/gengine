//
// VideoPlayer.cpp
//
// Clark Kromenaker
//
#include "VideoPlayer.h"

#include "Actor.h"
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
        
        // Video background size always equals desired size.
        mVideoBackground->GetRectTransform()->SetSizeDelta(mVideoSize);
        
        // If letterboxing, resize video image so as to not stretch the video.
        // The background will act as a "letterbox" if needed.
        if(mLetterbox && videoTexture != nullptr)
        {
            float videoWidth = videoTexture->GetWidth();
            float videoHeight = videoTexture->GetHeight();
            
            // Start by filling the width of the area.
            float widthRatio = mVideoSize.x / videoWidth;
            float newWidth = videoWidth * widthRatio;
            float newHeight = videoHeight * widthRatio;
            
            // If height is too large still for display area, scale again.
            if(newHeight > mVideoSize.y)
            {
                float heightRatio = mVideoSize.y / newHeight;
                newWidth *= heightRatio;
                newHeight *= heightRatio;
            }
            mVideoImage->GetRectTransform()->SetSizeDelta(Vector2(newWidth, newHeight));
        }
        else
        {
            // Video image just uses specified size, even if it doesn't match video's aspect ratio.
            // This means the video might be stretched and not correct aspect ratio.
            mVideoImage->GetRectTransform()->SetSizeDelta(mVideoSize);
        }
    }
}


void VideoPlayer::Play(const std::string& name)
{
    mVideo = new VideoState(name.c_str());
    
    // Show fullscreen.
    mLetterbox = false;
    mVideoPosition = Vector2::Zero;
    mVideoSize = Vector2(100, 384);
}

void VideoPlayer::Stop()
{
    delete mVideo;
}

