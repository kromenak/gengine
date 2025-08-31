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

namespace VideoHelper
{
    inline void PlayVideoWithCaptions(const std::string& videoName, bool fullscreen, bool autoclose, const std::function<void()>& callback)
    {
        // HACK: On a case-sensitive file system (like Linux), the video names may not work because case doesn't match.
        // HACK: So this map ensures we play the exact video file, with exact case.
        // HACK: Probably a better fix long-term is to make the asset loading functions more versatile to handle this scenario.
        static std::string_map_ci<std::string> videoNameToFilename = {
            { "202AEnd", "202aend.bik" },
            { "205PEnd", "205PEND.BIK" },
            { "207AEnd", "207aend.bik" },
            { "212PBegin", "212pBegin.bik" },
            { "212PEnd", "212PEND.BIK" },
            { "310ABegin", "310abegin.bik" },
            { "Day2-1", "day2-1.bik" },
            { "Day2-2", "day2-2.bik" },
            { "Day3-1", "day3-1.bik" },
            { "Day3-2", "day3-2.bik" },
            { "Day3-3", "day3-3.bik" },
            { "Day3-4", "day3-4.bik" },
            { "Day3-5", "day3-5.bik" },
            { "Day3-6", "day3-6.bik" },
            { "Day3-7", "day3-7.bik" },
            { "Day3-8", "day3-8.bik" },
            { "Day3-9", "day3-9.bik" },
            { "Day3-a", "day3-a.bik" },
            { "Day3-b", "day3-b.bik" },
        };

        // Try to map the video name to its exact filename - most important on Linux.
        // If not found, we fall back on just using the video name and hoping the asset loader can figure it out.
        std::string filename = videoName;
        auto it = videoNameToFilename.find(Path::GetFileNameNoExtension(videoName));
        if(it != videoNameToFilename.end())
        {
            filename = it->second;
        }

        // Play video.
        gGK3UI.GetVideoPlayer()->Play(filename, fullscreen, autoclose, [videoName, callback](){

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
        if(gGK3UI.GetVideoPlayer()->IsPlaying())
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