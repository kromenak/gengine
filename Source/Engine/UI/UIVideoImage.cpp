#include "UIVideoImage.h"

#include "AssetManager.h"
#include "Texture.h"
#include "VideoState.h"

UIVideoImage::UIVideoImage(Actor* owner) : UIImage(owner)
{

}

UIVideoImage::~UIVideoImage()
{
    // End any playing video (cleans up video memory, fires any outstanding callback).
    EndVideoPlayback();

    // If we have a video texture, we own it - delete it to avoid a memory leak.
    delete mVideoTexture;
}

bool UIVideoImage::Play(const std::string& videoName, const std::function<void()>& callback)
{
    // If a video was previously already playing, end that one first.
    EndVideoPlayback();

    // Save passed in callback.
    mCallback = callback;

    // The name passed is just a filename (e.g. "intro.bik"). Need to convert that into a full path.
    // Names can also be passed with or without extension, so we have to try to resolve any ambiguous name.
    std::string videoPath = gAssetManager.GetAssetPath(videoName, { "bik", "avi" });

    // Create new video.
    mVideo = new VideoState(videoPath.c_str());

    // On create, video begins playing immediately, assuming no error occurs.
    // If stopped, something happened, and video will not play. Clean up and get out of here.
    if(mVideo->IsStopped())
    {
        EndVideoPlayback();
        return false;
    }

    // If a video texture was previously saved, and we're now playing a new movie, delete the old video texture.
    if(mVideoTexture != nullptr)
    {
        delete mVideoTexture;
        mVideoTexture = nullptr;
    }

    // Cache new video texture.
    // This texture should also already contain the first frame of the video.
    mVideoTexture = mVideo->GetVideoTexture();

    // By default, the video system creates and owns the texture the video pixels get rendered to.
    // So when the video is done playing, and the Video is deleted, the video texture also gets deleted.
    //
    // However, in this use case, we want to keep the last video frame visible until the UIVideoImage gets destroyed.
    // As a result, we tell the video to relenquish ownership of the video texture so we can control its lifespan instead.
    mVideo->RelenquishVideoTextureOwnership();

    // Set texture to display on this image, resizing if needed.
    // This only needs to be set once, since the video system just keeps updating the texture's pixels!
    SetTexture(mVideoTexture, true);
    return true;
}

bool UIVideoImage::Play(const std::string& videoName, const Color32& transparentColor, const std::function<void()>& callback)
{
    // Use the passed in color as the transparent color in this video.
    SetTransparentColor(transparentColor);

    // Play per usual.
    return Play(videoName, callback);
}

void UIVideoImage::Stop()
{
    // Just end video playback prematurely.
    // If no video is playing, this should do nothing.
    EndVideoPlayback();
}

bool UIVideoImage::IsPlaying() const
{
    // Video is playing as long as non-null and not reporting as stopped.
    return mVideo != nullptr && !mVideo->IsStopped();
}

void UIVideoImage::OnUpdate(float deltaTime)
{
    if(mVideo != nullptr)
    {
        // Updating the video keeps new pixels flowing into the video texture each frame.
        mVideo->Update();

        // When the video stops (either naturally or prematurely), video playback is over.
        // Clean up and execute callback.
        if(mVideo->IsStopped())
        {
            EndVideoPlayback();
        }
    }
}

void UIVideoImage::EndVideoPlayback()
{
    // Delete any video that was playing.
    if(mVideo != nullptr)
    {
        delete mVideo;
        mVideo = nullptr;
    }

    // Fire video end callback.
    if(mCallback != nullptr)
    {
        auto callback = mCallback;
        mCallback = nullptr;
        callback();
    }
}
