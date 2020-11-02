//
// VideoPlaybackGL.cpp
//
// Clark Kromenaker
//
#include "VideoPlaybackGL.h"

extern "C"
{
    #include <libavutil/time.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}

#include "FrameQueue.h"
#include "VideoState.h"

#include "Mesh.h"
#include "Services.h"
#include "Texture.h"

extern Mesh* uiQuad;

VideoPlaybackGL::~VideoPlaybackGL()
{
    // Free conversion contexts.
    sws_freeContext(mRBGAConvertContext);
    //sws_freeContext(sub_convert_ctx);
}

void VideoPlaybackGL::Update(VideoState* is)
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
            if(!step
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
        if(step && !is->IsPaused())
        {
            is->TogglePause();
        }
    }
    
    /*
    // Clear frame queue of any obsolete frames.
    while(is->videoFrames.GetUndisplayedCount() > 0)
    {
        Frame* current = is->videoFrames.Peek();
        if(current->serial == is->videoPackets.serial)
        {
            break;
        }
        is->videoFrames.Dequeue();
    }
    
    // No undisplayed frames? Don't update, continue to show current frame.
    if(is->videoFrames.GetUndisplayedCount() == 0)
    {
        return;
    }
    
    // If playback is paused, no need to update.
    if(is->IsPaused())
    {
        return;
    }
    
    while(is->videoFrames.GetUndisplayedCount() > 0)
    {
        // If serial changed since last frame, reset frame timer.
        Frame* current = is->videoFrames.Peek();
        Frame* last = is->videoFrames.PeekLast();
        if(current->serial != last->serial)
        {
            is->frameTimer = PtsClock::GetCurrentTimeSeconds();
        }
        
        // Compute how much we want to delay showing a new video frame (for sync purposes).
        double delay = SyncVideo(is, CalculateFrameDuration(is, last, current));

        // If not enough time has passed to show the next frame, don't update texture.
        // Continue to show the same frame.
        double nextFrameShowTime = is->frameTimer + delay;
        double time = PtsClock::GetCurrentTimeSeconds();
        if(time < nextFrameShowTime)
        {
            
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
        if(!isnan(current->pts))
        {
            is->videoClock.SetPts(current->pts, current->serial);
            is->externalClock.SyncTo(&is->videoClock);
        }
        is->videoFrames.UnlockMutex();
        
        // If there are multiple undisplayed frames, see if the next one is "late".
        // "Late" meaning, the time at which we should have shown it has passed.
        // In that case, we skip the frame entirely (i.e. "drop it").
        if(is->videoFrames.GetUndisplayedCount() > 1)
        {
            Frame* next = is->videoFrames.PeekNext();
            if(!step
               && is->GetMasterSyncType() != AV_SYNC_VIDEO_MASTER
               && time > is->frameTimer + CalculateFrameDuration(is, current, next))
            {
                //is->frame_drops_late++; // useful for debug output
                is->videoFrames.Dequeue();
                continue;
            }
        }
    }
    */
}

bool VideoPlaybackGL::UpdateVideoTexture(Frame* videoFrame)
{
    // Already uploaded video texture to GPU - don't do it again.
    if(videoFrame->uploaded) { return true; }
    
    AVFrame* avFrame = videoFrame->frame;
    
    // Make sure we have a properly sized video texture.
    if(mVideoTexture == nullptr || mVideoTexture->GetWidth() != avFrame->width || mVideoTexture->GetHeight() != avFrame->height)
    {
        if(mVideoTexture != nullptr)
        {
            delete mVideoTexture;
        }
        mVideoTexture = new Texture(avFrame->width, avFrame->height);
    }
    
    // Create conversion context to go from input frame format to BGRA format.
    //TODO: There may be more efficient options for displaying a video frame. Ex: YUV420 data can be loaded into texture and rendered with special shader.
    //TODO: But for now, just convert all formats to RGBA for simplicity.
    mRBGAConvertContext = sws_getCachedContext(mRBGAConvertContext,
        avFrame->width, avFrame->height, (AVPixelFormat)avFrame->format, // from format
        avFrame->width, avFrame->height, AV_PIX_FMT_RGBA,              // to format
        SWS_BICUBIC, nullptr, nullptr, nullptr);
    if(mRBGAConvertContext == nullptr)
    {
        av_log(NULL, AV_LOG_FATAL, "Cannot initialize the conversion context\n");
        return false;
    }
    
    // Convert from source format to RGBA, copying RGBA data directly to Texture buffer.
    uint8_t* dest[4] { mVideoTexture->GetPixelData(), nullptr, nullptr, nullptr };
    int dest_linesize[4] = { static_cast<int>(mVideoTexture->GetWidth() * 4), 0, 0, 0 };
    sws_scale(mRBGAConvertContext,
              avFrame->data, avFrame->linesize, 0, avFrame->height, // source
              dest, dest_linesize); // dest
    
    // Upload texture data to GPU.
    mVideoTexture->UploadToGPU();
    
    // Yep, we are uploaded.
    videoFrame->uploaded = true;
    //videoFrame->flipVertical = avFrame->linesize[0] < 0; //TODO: Deal with vertical flip if needed
    return true;
}


void VideoPlaybackGL::UpdateSubtitles(VideoState* is)
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

double VideoPlaybackGL::CalculateFrameDuration(VideoState* is, Frame* from, Frame* to)
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

double VideoPlaybackGL::SyncVideo(VideoState* is, double delay)
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
    
    av_log(NULL, AV_LOG_TRACE, "video: delay=%0.3f A-V=%f\n", delay, -diff);
    return delay;
}






/*
// OLD CRAP
void VideoPlaybackGL::Render(VideoState* is)
{
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
            RenderInternal(is);
            return;
        }

        // Compute how much we want to delay showing a new video frame (for sync purposes).
        double delay = SyncVideo(is, CalculateFrameDuration(is, lastvp, vp));

        // If current time is less than when we'd want to show a new frame, skip to display (duplicate frame).
        double time = PtsClock::GetCurrentTimeSeconds();
        if(time < is->frameTimer + delay)
        {
            // *remaining_time = FFMIN(is->frame_timer + delay - time, *remaining_time);
            RenderInternal(is);
            return;
        }

        // If...
        is->frameTimer += delay;
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

        // See if we need to skip a frame.
        if(is->videoFrames.GetUndisplayedCount() > 1)
        {
            Frame* nextvp = is->videoFrames.PeekNext();
            if(!step
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
                // Grab next subtitle frame.
                Frame* sp = is->subtitleFrames.Peek();
                
                // Grab subtitle frame AFTER that (if any exists).
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
                        ClearSubtitleTexture();
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
        force_refresh = true;

        // If stepping and not paused...pause!
        if(step && !is->IsPaused())
        {
            is->TogglePause();
        }
    }
    
    // Render current frame. If above code dequeued from frame queue, we will see a new video frame.
    // If not, we'll see the same frame repeated.
    RenderInternal(is);
}

void VideoPlaybackGL::RenderInternal(VideoState* is)
{
    // Get video frame.
    Frame* videoFrame = is->videoFrames.PeekLast();
    Frame* subtitleFrame = nullptr;
    
    if(force_refresh && is->videoFrames.HasReadableFrame())
    {
        if(is->videoStream != nullptr)
        {
            // Get subtitle, if any.
            if(is->subtitleStream != nullptr && is->subtitleFrames.GetUndisplayedCount() > 0)
            {
                subtitleFrame = is->subtitleFrames.Peek();
                
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
                        
                        UpdateSubtitleTexture(subtitleFrame);
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
            
            // Upload video frame to GPU, if not yet uploaded.
            if(!videoFrame->uploaded)
            {
                // Upload video texture - fail out if this doesn't work.
                if(!UpdateVideoTexture(videoFrame))
                {
                    return;
                }
                videoFrame->uploaded = true;
                videoFrame->flipVertical = videoFrame->frame->linesize[0] < 0;
            }
        }
    }
    
    // Clear force refresh flag.
    force_refresh = false;
    
    RenderForReal(is, videoFrame, subtitleFrame);
}

void VideoPlaybackGL::RenderForReal(VideoState* is, Frame* vp, Frame* sp)
{
    mMaterial.SetDiffuseTexture(mVideoTexture);
    
    Vector2 windowSize = Services::GetRenderer()->GetWindowSize();
    Vector3 size(windowSize.x, windowSize.y, 1.0f);
    Vector3 pos(windowSize.x * 0.5f - size.x * 0.5f, windowSize.y * 0.5f - size.y * 0.5f, 0.0f);
    
    mMaterial.Activate(Matrix4::MakeTranslate(pos) * Matrix4::MakeScale(size));
    uiQuad->Render();
}
*/
