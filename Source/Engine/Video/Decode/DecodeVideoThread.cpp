//
// Clark Kromenaker
//
// Thread for decoding video data to the video frame queue.
//
#include "VideoState.h"

namespace
{
    int GetVideoFrame(VideoState* is, AVFrame* frame)
    {
        // Decode a frame.
        // Returns 1 if frame was decoded, 0 if no decoded frame, -1 on abort.
        int gotVideoFrame = is->videoDecoder.DecodeFrame(frame, nullptr);

        // Looks like aborting.
        if(gotVideoFrame < 0)
        {
            return -1;
        }

        // We got a picture, but we might purposely ignore it for sync purposes!
        if(gotVideoFrame > 0)
        {
            // If not synced to video, we need to check if this frame is needed based on the audio or external clocks.
            // We also need a valid pts value to do timestamp calculations - if not present, we can't tell whether we would want to drop this frame.
            if(is->GetMasterSyncType() != AV_SYNC_VIDEO_MASTER && frame->pts != AV_NOPTS_VALUE)
            {
                // Calculate how far ahead or behind this video frame is compared to the master clock.
                // Positive = this frame is in the future, negative = this frame is in the past.
                double dpts = av_q2d(is->videoStream->time_base) * frame->pts;
                double diff = dpts - is->GetMasterClock();

                // If the diff is within the sync threshold AND this frame is in the past, we actually don't need it!
                // The time to display this frame has passed, so we can drop it.
                if(!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
                   diff < 0 &&
                   is->videoDecoder.GetSerial() == is->videoClock.GetSerial() &&
                   is->videoPackets.GetPacketCount() > 0)
                {
                    //is->frame_drops_early++; // useful for debug output
                    av_frame_unref(frame);
                    gotVideoFrame = 0;
                }
            }
        }
        return gotVideoFrame;
    }
}

int DecodeVideoThread(void* arg)
{
    VideoState* is = static_cast<VideoState*>(arg);

    // Allocate frame or fail.
    AVFrame* avFrame = av_frame_alloc();
    if(avFrame == nullptr)
    {
        return AVERROR(ENOMEM);
    }

    // Guess frame rate and cache it.
    AVRational frame_rate = av_guess_frame_rate(is->format, is->videoStream, nullptr);

    AVRational avRational;
    avRational.den = frame_rate.den;
    avRational.num = frame_rate.num;
    double duration = (frame_rate.num && frame_rate.den ? av_q2d(avRational) : 0);

    // Loop, decoding frames and putting them in the frame queue.
    while(true)
    {
        // Decode video frame (and also do some sync check stuff).
        int got_frame = GetVideoFrame(is, avFrame);
        if(got_frame < 0)
        {
            goto the_end;
        }

        // Ok, decoded a frame.
        if(got_frame > 0)
        {
            // Populate AVFrame's aspect ratio using best guess.
            avFrame->sample_aspect_ratio = av_guess_sample_aspect_ratio(is->format, is->videoStream, avFrame);

            // Get next frame from frame queue, or fail.
            Frame* vp = is->videoFrames.PeekWritable();
            if(vp == nullptr)
            {
                goto the_end;
            }

            // Pass along serial.
            vp->serial = is->videoDecoder.GetSerial();

            // Store pts/pos/duration.
            vp->pts = (avFrame->pts == AV_NOPTS_VALUE) ? std::numeric_limits<double>::quiet_NaN() : avFrame->pts * av_q2d(is->videoStream->time_base);
            vp->pos = avFrame->pkt_pos;
            vp->duration = duration;

            // Store aspect ratio, width, height.
            vp->aspectRatio = avFrame->sample_aspect_ratio;
            vp->width = avFrame->width;
            vp->height = avFrame->height;

            // Subtitles need to be uploaded to GPU.
            vp->uploaded = false;

            // Store AVFrame.
            av_frame_move_ref(vp->frame, avFrame);

            // Enqueue peeked frame to queue (makes it available for playback).
            is->videoFrames.Enqueue();
        }
    }

the_end:
    av_frame_free(&avFrame);
    return 0;
}
