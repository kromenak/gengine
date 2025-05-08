//
// Clark Kromenaker
//
// Plays videos but also adds support for captions using GK3-specific systems.
//
#pragma once
#include <functional>

#include "AssetManager.h"
#include "Animator.h"
#include "FileSystem.h"
#include "GK3UI.h"
#include "Scene.h"
#include "VideoPlayer.h"

namespace VideoHelper
{
    inline void PlayVideoWithCaptions(const std::string& videoName, bool fullscreen, bool autoclose, const std::function<void()>& callback)
    {
        // Play video.
        gVideoPlayer.Play(videoName, fullscreen, autoclose, [videoName, callback](){

            // If the video ends naturally, any associated captions will also end naturally.
            // But in case of skipping the video, be sure to stop captions prematurely as well.
            Animation* captionsAnim = gAssetManager.LoadYak(Path::GetFileNameNoExtension(videoName), AssetScope::Scene);
            if(captionsAnim != nullptr)
            {
                Scene::GetGlobalAnimator()->Stop(captionsAnim);
            }

            // Similarly, if video ends naturally, captions will disappear on their own.
            // But when skipping the video, we want to ensure captions hide right away.
            gGK3UI.HideAllCaptions();

            // Execute callback.
            if(callback != nullptr)
            {
                callback();
            }
        });

        // If video is playing, attempt to play associated captions.
        if(gVideoPlayer.IsPlaying())
        {
            Animation* captionsAnim = gAssetManager.LoadYak(Path::GetFileNameNoExtension(videoName), AssetScope::Scene);
            if(captionsAnim != nullptr)
            {
                AnimParams yakAnimParams;
                yakAnimParams.animation = captionsAnim;
                yakAnimParams.isYak = true;
                Scene::GetGlobalAnimator()->Start(yakAnimParams);
            }
        }
    }

    inline void PlayVideoWithCaptions(const std::string& videoName, const std::function<void()>& callback)
    {
        PlayVideoWithCaptions(videoName, true, true, callback);
    }
}