#include "VideoPlayback.h"

extern "C"
{
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}

#include "FrameQueue.h"
#include "Texture.h"
#include "VideoState.h"

VideoPlayback::~VideoPlayback()
{
    // Free conversion contexts.
    sws_freeContext(mRGBAConvertContext);
    //sws_freeContext(sub_convert_ctx);

    // Free video texture.
    if(mOwnsVideoTexture && mVideoTexture != nullptr)
    {
        delete mVideoTexture;
    }
}

void VideoPlayback::Update(VideoState* is)
{
    // GOAL: update video texture with appropriate data for current playback state.
    // No video stream? Can't do anything.
    if(is->videoStream == nullptr)
    {
        return;
    }

    //double remaining = 0.0;
    //double* remaining_time = &remaining;
retry:
    if(is->videoFrames.GetUndisplayedCount() == 0)
    {
        // nothing to do, no picture to display in the queue
    }
    else
    {
        // Get current and last frames.
        // "Last" is needed primarily to detect serial change and calc time diff between frames.
        Frame* lastvp = is->videoFrames.PeekLast();
        Frame* vp = is->videoFrames.Peek();

        // If serials don't match, skip frame.
        if(vp->serial != is->videoPackets.serial)
        {
            is->videoFrames.Dequeue();
            goto retry;
        }

        // If serial changed since last frame, reset frame timer.
        if(lastvp->serial != vp->serial)
        {
            is->frameTimer = PtsClock::GetCurrentTimeSeconds();
        }

        // If playback is paused, go directly to display.
        // Do not take a new frame for playback, just keep showing the current one.
        if(is->IsPaused())
        {
            return;
        }

        // Compute how much we want to delay showing a new video frame (for sync purposes).
        double delay = SyncVideo(is, CalculateFrameDuration(is, lastvp, vp));

        // If current time is less than when we'd want to show a new frame, skip to display (duplicate frame).
        double time = PtsClock::GetCurrentTimeSeconds();
        if(time < is->frameTimer + delay)
        {
            //*remaining_time = FFMIN(is->frame_timer + delay - time, *remaining_time);
            //RenderInternal(is);
            return;
        }

        // Increase frame timer by delay.
        is->frameTimer += delay;

        // Reset frame timer if beyond sync threshold.
        if(delay > 0 && time - is->frameTimer > AV_SYNC_THRESHOLD_MAX)
        {
            is->frameTimer = time;
        }

        // Update video clock.
        //TODO: Why do we need to lock the mutex here?
        is->videoFrames.LockMutex();
        if(!isnan(vp->pts))
        {
            is->videoClock.SetPts(vp->pts, vp->serial);
            is->externalClock.SyncTo(&is->videoClock);
        }
        is->videoFrames.UnlockMutex();

        // If there are multiple undisplayed frames, see if the next one is "late".
        // "Late" meaning, the time at which we should have shown it has passed.
        // In that case, we skip the frame entirely (i.e. "drop it").
        if(is->videoFrames.GetUndisplayedCount() > 1)
        {
            Frame* nextvp = is->videoFrames.PeekNext();
            if(!mStep
               && is->GetMasterSyncType() != AV_SYNC_VIDEO_MASTER
               && time > is->frameTimer + CalculateFrameDuration(is, vp, nextvp))
            {
                //is->frame_drops_late++; // useful for debug output
                is->videoFrames.Dequeue();
                goto retry;
            }
        }

        // See if there's a new subtitle frame we should be showing.
        // And if so, update the subtitle texture.
        if(is->subtitleStream != nullptr)
        {
            while(is->subtitleFrames.GetUndisplayedCount() > 0)
            {
                // Grab current subtitle frame.
                Frame* sp = is->subtitleFrames.Peek();

                // Grab next subtitle frame.
                Frame* sp2 = nullptr;
                if(is->subtitleFrames.GetUndisplayedCount() > 1)
                {
                    sp2 = is->subtitleFrames.PeekNext();
                }

                // If subtitle frame has incorrect serial OR
                // video clock is beyond end time OR
                // video clock is beyond next subtitle frame's start time...skip this subtitle frame!
                if(sp->serial != is->subtitlePackets.serial ||
                   (is->videoClock.GetPts() > (sp->pts + ((float) sp->sub.end_display_time / 1000))) ||
                   (sp2 != nullptr && is->videoClock.GetPts() > (sp2->pts + ((float) sp2->sub.start_display_time / 1000))))
                {
                    // If subtitle frame has been previously uploaded, we need to get rid of it.
                    // So, we should clear the pixels in the subtitle texture so we aren't showing a stale subtitle.
                    if(sp->uploaded)
                    {
                        //TODO: Clear subtitle texture!
                        //ClearSubtitleTexture();
                    }
                    is->subtitleFrames.Dequeue();
                }
                else
                {
                    // Subtitle frame is fit to show, so break out of loop.
                    break;
                }
            }
        }

        // We're going to show a new video frame!
        is->videoFrames.Dequeue();
        //force_refresh = true;

        // Got here with readable frame means...update video texture!
        if(is->videoFrames.HasReadableFrame())
        {
            UpdateVideoTexture(is->videoFrames.PeekLast());
            UpdateSubtitles(is);
        }

        // If stepping and not paused...pause!
        if(mStep && !is->IsPaused())
        {
            is->TogglePause();
        }
    }
}

void VideoPlayback::SetTransparentColor(const Color32& color)
{
    // Save transparent color to apply to future generated video pixels.
    mHasTransparentColor = true;
    mTransparentColor = color;

    // If a texture already exists, also update its existing video pixels.
    if(mVideoTexture != nullptr)
    {
        mVideoTexture->SetTransparentColor(color);
    }
}

void VideoPlayback::ClearTransparentColor()
{
    mHasTransparentColor = false;
}

bool VideoPlayback::UpdateVideoTexture(Frame* videoFrame)
{
    // Already uploaded video texture to GPU - don't do it again.
    if(videoFrame->uploaded) { return true; }

    // Make sure we have a properly sized video texture.
    AVFrame* avFrame = videoFrame->frame;
    if(mVideoTexture == nullptr)
    {
        mVideoTexture = new Texture(avFrame->width, avFrame->height);
    }
    else if(mVideoTexture->GetWidth() != avFrame->width || mVideoTexture->GetHeight() != avFrame->height)
    {
        mVideoTexture->Resize(avFrame->width, avFrame->height);
    }

    // Create conversion context to go from input frame format to BGRA format.
    //TODO: There may be more efficient options for displaying a video frame. Ex: YUV420 data can be loaded into texture and rendered with special shader.
    //TODO: But for now, just convert all formats to RGBA for simplicity.
    mRGBAConvertContext = sws_getCachedContext(mRGBAConvertContext,
        avFrame->width, avFrame->height, (AVPixelFormat)avFrame->format,    // from format
        avFrame->width, avFrame->height, AV_PIX_FMT_RGBA,                   // to format
        SWS_BICUBIC, nullptr, nullptr, nullptr);
    if(mRGBAConvertContext == nullptr)
    {
        av_log(NULL, AV_LOG_FATAL, "Cannot initialize the conversion context\n");
        return false;
    }

    // Convert from source format to RGBA, copying RGBA data directly to Texture buffer.
    uint8_t* dest[4] { mVideoTexture->GetPixelData(), nullptr, nullptr, nullptr };
    int dest_linesize[4] = { static_cast<int>(mVideoTexture->GetWidth() * 4), 0, 0, 0 };
    sws_scale(mRGBAConvertContext,
              avFrame->data, avFrame->linesize, 0, avFrame->height, // source
              dest, dest_linesize); // dest

    // After pixels are updated in texture, apply transparent color if any.
    if(mHasTransparentColor)
    {
        mVideoTexture->SetTransparentColor(mTransparentColor);
    }

    // Upload texture data to GPU.
    mVideoTexture->AddDirtyFlags(Texture::DirtyFlags::Pixels);
    mVideoTexture->UploadToGPU();

    // Yep, we are uploaded.
    videoFrame->uploaded = true;
    //videoFrame->flipVertical = avFrame->linesize[0] < 0; //TODO: Deal with vertical flip if needed
    return true;
}

void VideoPlayback::UpdateSubtitles(VideoState* is)
{
    // See if there's a new subtitle frame we should be showing.
    // And if so, update the subtitle texture.
    if(is->subtitleStream != nullptr)
    {
        while(is->subtitleFrames.GetUndisplayedCount() > 0)
        {
            // Grab current subtitle frame.
            Frame* sp = is->subtitleFrames.Peek();

            // Grab next subtitle frame.
            Frame* sp2 = nullptr;
            if(is->subtitleFrames.GetUndisplayedCount() > 1)
            {
                sp2 = is->subtitleFrames.PeekNext();
            }

            // If subtitle frame has incorrect serial OR
            // video clock is beyond end time OR
            // video clock is beyond next subtitle frame's start time...skip this subtitle frame!
            if(sp->serial != is->subtitlePackets.serial ||
               (is->videoClock.GetPts() > (sp->pts + ((float) sp->sub.end_display_time / 1000))) ||
               (sp2 != nullptr && is->videoClock.GetPts() > (sp2->pts + ((float) sp2->sub.start_display_time / 1000))))
            {
                // If subtitle frame has been previously uploaded, we need to get rid of it.
                // So, we should clear the pixels in the subtitle texture so we aren't showing a stale subtitle.
                if(sp->uploaded)
                {
                    //TODO: Clear subtitle texture pixels!
                }
                is->subtitleFrames.Dequeue();
            }
            else
            {
                // Subtitle frame is fit to show, so break out of loop.
                break;
            }
        }
    }

    // Get subtitle, if any.
    if(is->subtitleStream != nullptr && is->subtitleFrames.GetUndisplayedCount() > 0)
    {
        Frame* videoFrame = is->videoFrames.PeekLast();
        Frame* subtitleFrame = is->subtitleFrames.Peek();

        // If video pts is ahead of subtitle pts, we should show this subtitle!
        if(videoFrame->pts >= subtitleFrame->pts + ((float)subtitleFrame->sub.start_display_time / 1000))
        {
            // Upload subtitle frame texture to GPU if needed.
            if(!subtitleFrame->uploaded)
            {
                // Make sure width/height are set.
                if(subtitleFrame->width == 0 ||
                   subtitleFrame->height == 0)
                {
                    subtitleFrame->width = videoFrame->width;
                    subtitleFrame->height = videoFrame->height;
                }

                //TODO: Update subtitle texture pixels!

                subtitleFrame->uploaded = true;
            }
        }
        else
        {
            // The subtitle is for a video frame we have not yet shown.
            // So, ignore it for now.
            subtitleFrame = nullptr;
        }
    }
}

double VideoPlayback::CalculateFrameDuration(VideoState* is, Frame* from, Frame* to)
{
    // Frames are not continuous, so can't calculate duration.
    if(from->serial != to->serial)
    {
        return 0.0;
    }

    // Max frame duration depends on whether format allows timestamp discontinuities.
    // If discontinuities are allowed, we can use a small max frame duration.
    double max_frame_duration = (is->format->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

    // In theory, duration of a frame is diff between sequential frame pts.
    // But if that value doesn't make sense, fall back on estimated duration (calculated from video frame rate).
    double duration = to->pts - from->pts;
    if(isnan(duration) || duration <= 0 || duration > max_frame_duration)
    {
        // Using estimated duration.
        return from->duration;
    }
    else
    {
        // Using calculated duration.
        return duration;
    }
}

double VideoPlayback::SyncVideo(VideoState* is, double delay)
{
    // If video is master, it gets to do what it wants - use the delay directly.
    if(is->GetMasterSyncType() == AV_SYNC_VIDEO_MASTER)
    {
        return delay;
    }

    // Video is not master, so it is synced to audio or an external clock.
    // If video is slave, try to correct big delays by duplicating or deleting a frame.

    // Calculate how far or behind video clock is from master clock.
    // Positive = video is ahead, negative = video is behind.
    double diff = is->videoClock.GetTime() - is->GetMasterClock();

    // Max frame duration depends on whether format allows timestamp discontinuities.
    // If discontinuities are allowed, we can use a small max frame duration.
    double max_frame_duration = (is->format->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;
    if(!isnan(diff) && fabs(diff) < max_frame_duration)
    {
        /* skip or repeat frame. We take into account the
           delay to compute the threshold. I still don't know
           if it is the best guess */
        double sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
        if(diff <= -sync_threshold)
        {
            delay = FFMAX(0, delay + diff);
        }
        else if(diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
        {
            delay = delay + diff;
        }
        else if(diff >= sync_threshold)
        {
            delay = 2 * delay;
        }
    }

    //av_log(NULL, AV_LOG_TRACE, "video: delay=%0.3f A-V=%f\n", delay, -diff);
    return delay;
}
