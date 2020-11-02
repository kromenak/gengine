//
// ReadThread.cpp
//
// Clark Kromenaker
//
// Video read thread function. Broken out into its own file b/c it's kind of its own thing.
//
// Reads data from video file and puts into appropriate packet queue for further processing.
// In other words, performs "demuxing" of video data.
//
#include "VideoState.h"

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25

static int stream_has_enough_packets(AVStream *st, int stream_id, PacketQueue* queue) {
    return stream_id < 0 ||
           queue->Aborted() ||
           queue->GetPacketCount() > (MIN_FRAMES && (!queue->GetDuration() || av_q2d(st->time_base) * queue->GetDuration() > 1.0));
}

void ReadThreadFinished(SDL_mutex* waitMutex, int ret)
{
    // Clean up wait mutex.
    SDL_DestroyMutex(waitMutex);
    
    //TODO: "ret" can tell us why read thread finished (abort, eof, fatal error, etc).
}

/* this thread gets the stream from the disk or the network */
/*static*/ int VideoState::ReadThread(void* arg)
{
    VideoState* is = static_cast<VideoState*>(arg);
    
    // Create wait mutex or fail.
    SDL_mutex* wait_mutex = SDL_CreateMutex();
    if(wait_mutex == nullptr)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        ReadThreadFinished(wait_mutex, AVERROR(ENOMEM));
        return 0;
    }
    
    // Loop indefinitely, reading in data from input streams to packet queues.
    AVPacket avPacket;
    while(true)
    {
        // Abort was requested, so break out.
        if(is->mAborted) { break; }
        
        // Seek requested.
        if(is->seek_req)
        {
            int64_t seek_target = is->seek_pos;
            int64_t seek_min    = is->seek_rel > 0 ? seek_target - is->seek_rel + 2: INT64_MIN;
            int64_t seek_max    = is->seek_rel < 0 ? seek_target - is->seek_rel - 2: INT64_MAX;
            // FIXME the +-2 is due to rounding being not done in the correct direction in generation of the seek_pos/seek_rel variables

            int ret = avformat_seek_file(is->format, -1, seek_min, seek_target, seek_max, is->seek_flags);
            if(ret < 0)
            {
                av_log(NULL, AV_LOG_ERROR, "%s: error while seeking\n", is->format->url);
            }
            else
            {
                if(is->mAudioStreamIndex >= 0)
                {
                    is->audioPackets.Clear();
                    is->audioPackets.Enqueue(&gFlushPacket);
                }
                if(is->mSubtitleStreamIndex >= 0)
                {
                    is->subtitlePackets.Clear();
                    is->subtitlePackets.Enqueue(&gFlushPacket);
                }
                if(is->mVideoStreamIndex >= 0)
                {
                    is->videoPackets.Clear();
                    is->videoPackets.Enqueue(&gFlushPacket);
                }
                if(is->seek_flags & AVSEEK_FLAG_BYTE)
                {
                    is->externalClock.SetPts(NAN, 0);
                }
                else
                {
                    is->externalClock.SetPts(seek_target / (double)AV_TIME_BASE, 0);
                }
            }
            is->seek_req = 0;
            is->mEOF = false;
            if(is->mPaused)
            {
                is->StepToNextFrame();
            }
        }

        // If the queue are full, no need to read more.
        bool tooMuchData = is->audioPackets.GetByteSize() +
                           is->videoPackets.GetByteSize() +
                           is->subtitlePackets.GetByteSize() > MAX_QUEUE_SIZE;
        bool allStreamsHappy = stream_has_enough_packets(is->audioStream, is->mAudioStreamIndex, &is->audioPackets) &&
                               stream_has_enough_packets(is->videoStream, is->mVideoStreamIndex, &is->videoPackets) &&
                               stream_has_enough_packets(is->subtitleStream, is->mSubtitleStreamIndex, &is->subtitlePackets);
        if(tooMuchData || allStreamsHappy)
        {
            if(tooMuchData)
            {
                SDL_Log("Too much data - sleeping for 10ms");
            }
            
            // Wait 10 ms, or until continue read condition is signaled
            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->mContinueReadCondition, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            
            // Everything below this point is about reading/decoding packets, but we have too many packets!
            // So, loop back to top.
            continue;
        }
        
        // Read all the data - end thread.
        if(!is->mPaused &&
           (is->audioStream == nullptr || (is->audioDecoder.ReachedEOF() && is->audioFrames.GetUndisplayedCount() == 0)) &&
           (is->videoStream == nullptr || (is->videoDecoder.ReachedEOF() && is->videoFrames.GetUndisplayedCount() == 0))) {
            ReadThreadFinished(wait_mutex, AVERROR_EOF);
            return 0;
        }
        
        // Attempt to read the next packet.
        int ret = av_read_frame(is->format, &avPacket);
        if(ret < 0)
        {
            // If reached end of file, enqueue null packets and set eof flag.
            if((ret == AVERROR_EOF || avio_feof(is->format->pb)) && !is->mEOF)
            {
                if(is->mVideoStreamIndex >= 0)
                {
                    is->videoPackets.EnqueueNull(is->mVideoStreamIndex);
                }
                if(is->mAudioStreamIndex >= 0)
                {
                    is->audioPackets.EnqueueNull(is->mAudioStreamIndex);
                }
                if(is->mSubtitleStreamIndex >= 0)
                {
                    is->subtitlePackets.EnqueueNull(is->mSubtitleStreamIndex);
                }
                is->mEOF = true;
            }
            
            // Some sort of playback error?
            if(is->format->pb && is->format->pb->error)
            {
                //TODO: Probably act like playback finished, but with an error log.
                break;
            }
            
            // Wait 10 ms, or until continue read condition is signaled.
            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->mContinueReadCondition, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        }
        else
        {
            // Read frame successfully, so guess it's not eof yet.
            is->mEOF = false;
        }
        
        // Queue packets for audio, video, and subtitles.
        // If a packet is not from one of those streams...clean it up and don't use it.
        if(avPacket.stream_index == is->mAudioStreamIndex)
        {
            is->audioPackets.Enqueue(&avPacket);
        }
        else if(avPacket.stream_index == is->mVideoStreamIndex)
        {
            is->videoPackets.Enqueue(&avPacket);
        }
        else if(avPacket.stream_index == is->mSubtitleStreamIndex)
        {
            is->subtitlePackets.Enqueue(&avPacket);
        }
        else
        {
            // Packet is not for any stream we're interested in, so it won't be used.
            // Unref it right away.
            av_packet_unref(&avPacket);
        }
    } // while(true)
    
    ReadThreadFinished(wait_mutex, 0);
    return 0;
}
