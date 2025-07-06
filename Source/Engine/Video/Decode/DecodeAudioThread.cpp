//
// Clark Kromenaker
//
// Thread for decoding audio data to the audio frame queue.
//
#include "VideoState.h"

int DecodeAudioThread(void* arg)
{
    VideoState* is = static_cast<VideoState*>(arg);

    // Allocate frame or fail.
    AVFrame* avFrame = av_frame_alloc();
    if(avFrame == nullptr)
    {
        return AVERROR(ENOMEM);
    }

    // Loop, decoding frames and putting them in the frame queue.
    int ret = 0;
    while(ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        // Decode a frame.
        // Returns 1 if frame was decoded, 0 if no decoded frame, -1 on abort.
        int got_frame = is->audioDecoder.DecodeFrame(avFrame, nullptr);

        // Looks like aborting.
        if(got_frame < 0)
        {
            goto the_end;
        }

        // Ok, decoded a frame.
        if(got_frame > 0)
        {
            // Get next frame from frame queue, or fail.
            Frame* af = is->audioFrames.PeekWritable();
            if(af == nullptr)
            {
                goto the_end;
            }

            // Pass along serial.
            af->serial = is->audioDecoder.GetSerial();

            // Store PTS/pos/duration.
            AVRational timeBase { 1, avFrame->sample_rate };
            af->pts = (avFrame->pts == AV_NOPTS_VALUE) ? std::numeric_limits<double>::quiet_NaN() : avFrame->pts * av_q2d(timeBase);
            af->pos = avFrame->pkt_pos;

            AVRational frameRational { avFrame->nb_samples, avFrame->sample_rate };
            af->duration = av_q2d(frameRational);

            // Store AVFrame.
            av_frame_move_ref(af->frame, avFrame);

            // Enqueue peeked frame to queue (makes it available for playback).
            is->audioFrames.Enqueue();
        }
    }

the_end:
    av_frame_free(&avFrame);
    return ret;
}
