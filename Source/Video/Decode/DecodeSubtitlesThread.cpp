//
// Clark Kromenaker
//
// Thread for decoding subtitle data to the subtitle frame queue.
//
#include "VideoState.h"

int DecodeSubtitlesThread(void* arg)
{
    VideoState* is = static_cast<VideoState*>(arg);

    // Loop, decoding subtitles and putting in frame queue.
    while(true)
    {
        // Get next frame from frame queue, or fail.
        Frame* sp = is->subtitleFrames.PeekWritable();
        if(sp == nullptr)
        {
            return 0;
        }

        // Try to decode a subtitle.
        // Returns 1 if decoded, 0 if not decoded, -1 on abort.
        int got_subtitle = is->subtitleDecoder.DecodeFrame(nullptr, &sp->sub);
        if(got_subtitle < 0)
        {
            break;
        }

        if(got_subtitle && sp->sub.format == 0)
        {
            // Pass along serial.
            sp->serial = is->subtitleDecoder.GetSerial();

            // Store pts.
            double pts = 0.0;
            if(sp->sub.pts != AV_NOPTS_VALUE)
            {
                pts = sp->sub.pts / (double)AV_TIME_BASE;
            }
            sp->pts = pts;

            // Save width/height of subtitles.
            AVCodecContext* avctx = is->subtitleDecoder.GetCodecContext();
            sp->width = avctx->width;
            sp->height = avctx->height;

            // Subtitles need to be uploaded to GPU.
            sp->uploaded = false;

            // Enqueue frame that was peeked.
            // Indicates that our peeked frame is ready for playback.
            is->subtitleFrames.Enqueue();
        }
        else if(got_subtitle)
        {
            // Since not queued for playback, free it right now.
            avsubtitle_free(&sp->sub);
        }
    }
    return 0;
}
