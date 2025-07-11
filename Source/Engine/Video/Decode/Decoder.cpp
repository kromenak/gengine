#include "Decoder.h"

#include "FrameQueue.h"
#include "PacketQueue.h"
#include "VideoState.h"

int DecodeAudioThread(void* arg);       // DecodeAudioThread.cpp
int DecodeVideoThread(void* arg);       // DecodeVideoThread.cpp
int DecodeSubtitlesThread(void* arg);   // DecodeSubtitlesThread.cpp

void Decoder::Init(AVCodecContext* codecContext, PacketQueue* packetQueue, SDL_cond* emptyQueueCond)
{
    mCodecContext = codecContext;
    mPacketQueue = packetQueue;
    mEmptyQueueCondition = emptyQueueCond;

    // Zero out ffmpeg structs (otherwise, could crash during cleanup due to garbage pointers).
    memset(&mPendingPacket, 0, sizeof(AVPacket));
    memset(&mFlushPtsTimeBase, 0, sizeof(AVRational));
    memset(&mNextPtsTimeBase, 0, sizeof(AVRational));
}

void Decoder::Destroy()
{
    av_packet_unref(&mPendingPacket);
    avcodec_free_context(&mCodecContext);
}

int Decoder::Start(VideoState* is)
{
    // Start packet queue.
    mPacketQueue->Start();

    // Start appropriate decode thread.
    switch(mCodecContext->codec_type)
    {
    case AVMEDIA_TYPE_VIDEO:
        mDecoderThread = SDL_CreateThread(DecodeVideoThread, "Decoder::DecodeVideo", is);
        break;
    case AVMEDIA_TYPE_AUDIO:
        mDecoderThread = SDL_CreateThread(DecodeAudioThread, "Decoder::DecodeAudio", is);
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        mDecoderThread = SDL_CreateThread(DecodeAudioThread, "Decoder::DecodeSubtitles", is);
        break;
    default:
        av_log(NULL, AV_LOG_ERROR, "Unexpected codec type!");
        break;
    }

    // Fail if couldn't create thread.
    if(mDecoderThread == nullptr)
    {
        av_log(NULL, AV_LOG_ERROR, "SDL_CreateThread(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }

    // All good.
    return 0;
}

void Decoder::Abort(FrameQueue* frameQueue)
{
    mPacketQueue->Abort();
    frameQueue->Signal();

    // Wait for the decoder thread to abort.
    SDL_WaitThread(mDecoderThread, nullptr);
    mDecoderThread = nullptr;

    mPacketQueue->Clear();
}

// This function is called by the decoder thread created in the "Start" function.
int Decoder::DecodeFrame(AVFrame* frame, AVSubtitle* sub)
{
    int ret = AVERROR(EAGAIN);

    // Loop until one of the following occurs:
    // 1) A frame is decoded, which populates the frame out variable.
    // 2) An abort is detected.
    while(true)
    {
        // Receive decoded data!
        // It's only worthwhile to get decoded data if packet being decoded has same serial as queue.
        // If not, the packet being decoded is no longer useful!
        if(mPacketQueue->serial == mSerial)
        {
            // Loop to receive an audio/video frame.
            // The loop aborts and function returns if: decorder aborts, reached EOF, or frame was decoded.
            // The loop exists and function continues if: decoder returns EAGAIN.
            do
            {
                // Handle abort request.
                if(mPacketQueue->Aborted()) { return -1; }

                // Attempt to receive a frame.
                // Returns 0 when a frame is returned.
                // Returns EAGAIN when there's no data to receive (meaning we must send it some packets first).
                ret = avcodec_receive_frame(mCodecContext, frame);

                // Get a decoded frame!
                if(ret >= 0)
                {
                    // Depending on codec type, do some work to set frame's pts correctly.
                    if(mCodecContext->codec_type == AVMEDIA_TYPE_VIDEO)
                    {
                        // For video, just use the best effort timestamp.
                        // (ffplay has other options here, but I don't think they are necessary to support)
                        frame->pts = frame->best_effort_timestamp;
                    }
                    else if(mCodecContext->codec_type == AVMEDIA_TYPE_AUDIO)
                    {
                        // Set audio pts either by rescaling the frame's pts OR cached "next pts" (if frame has no pts set).
                        AVRational timeBase { 1, frame->sample_rate };
                        if(frame->pts != AV_NOPTS_VALUE)
                        {
                            frame->pts = av_rescale_q(frame->pts, mCodecContext->pkt_timebase, timeBase);
                        }
                        else if(mNextPts != AV_NOPTS_VALUE)
                        {
                            frame->pts = av_rescale_q(mNextPts, mNextPtsTimeBase, timeBase);
                        }

                        // Save "next pts" value based on this frame's pts (if frame has pts set).
                        if(frame->pts != AV_NOPTS_VALUE)
                        {
                            mNextPts = frame->pts + frame->nb_samples;
                            mNextPtsTimeBase = timeBase;
                        }
                    }
                }

                // If EOF is returned, set EOF serial, flush, and return.
                // NOTE: In practice, I have not ever seen this occur. See "IsEofPacket" below for fallback.
                if(ret == AVERROR_EOF)
                {
                    mEofSerial = mSerial;
                    avcodec_flush_buffers(mCodecContext);
                    return 0;
                }

                // Received a decoded frame - return so it can be used.
                if(ret >= 0)
                {
                    return 1;
                }
            } while(ret != AVERROR(EAGAIN)); // Loop until avcodec_receive_frame returns EAGAIN.
                                             // This signifies we must send data to be decoded (done next).
        }

        // Get a packet to send to the decoder.
        // This is a loop b/c we'll keep grabbing packets until we find a valid one.
        AVPacket avPacket;
        while(true)
        {
            // If queue is empty, signal (tells reader thread to provide more data plz).
            if(mPacketQueue->GetPacketCount() == 0)
            {
                SDL_CondSignal(mEmptyQueueCondition);
            }

            // If a pending packet exists, try to use that.
            // Otherwise, grab packet from queue.
            if(mPacketPending)
            {
                av_packet_move_ref(&avPacket, &mPendingPacket);
                mPacketPending = false;
            }
            else if(mPacketQueue->Dequeue(true, &avPacket, &mSerial) < 0)
            {
                // Abort! "Get" only returns < 0 on abort.
                return -1;
            }

            // If retrieved packet's serial matches queue's serial, this is a valid packet!
            // Break out of while loop (drop down to send packet to decoder).
            if(mPacketQueue->serial == mSerial)
            {
                break;
            }

            // Got a packet, but serials don't match - this packet is stale.
            // Unref it, get it outta here!
            av_packet_unref(&avPacket);
        }

        // Determine what to do with the packet.
        if(PacketQueue::IsFlushPacket(&avPacket))
        {
            // If it's a flush packet, clear buffers and reset some data.
            // This only occurs during a seek operation.
            avcodec_flush_buffers(mCodecContext);
            mEofSerial = 0;
            mNextPts = mFlushPts;
            mNextPtsTimeBase = mFlushPtsTimeBase;
        }
        else if(PacketQueue::IsEofPacket(&avPacket))
        {
            // Received EOF packet, so we're done!
            mEofSerial = mSerial;
            avcodec_flush_buffers(mCodecContext);
            return 0;
        }
        else
        {
            // This is a real packet, so send it to be decoded!

            // Decoding works a bit differently if it's a subtitle.
            if(mCodecContext->codec_type == AVMEDIA_TYPE_SUBTITLE)
            {
                // Decodes a subtitle from a packet and stores the result in "sub".
                // "got_subtitle" is non-zero if a subtitle was decompressed.
                int got_subtitle = 0;
                ret = avcodec_decode_subtitle2(mCodecContext, sub, &got_subtitle, &avPacket);

                // Treat any error as "try again."
                if(ret < 0)
                {
                    ret = AVERROR(EAGAIN);
                }
                else
                {
                    // A null data AVPacket indicates that we want to flush the subtitle decoder.
                    // We should keep reusing this packet until no subtitle is gotten (all data is flushed).
                    if(got_subtitle != 0 && avPacket.data == nullptr)
                    {
                        mPacketPending = true;
                        av_packet_move_ref(&mPendingPacket, &avPacket);
                    }

                    // Got a subtitle? No problem.
                    // No subtitle but got data? Try again.
                    // No subtitle and no data? EOF.
                    if(got_subtitle != 0)
                    {
                        ret = 0;
                    }
                    else
                    {
                        ret = avPacket.data != nullptr ? AVERROR(EAGAIN) : AVERROR_EOF;
                    }
                }
            }
            else
            {
                // Send packet to the decoder.
                // If it tells us EAGAIN for some reason, put as pending packet to try again on next loop.
                if(avcodec_send_packet(mCodecContext, &avPacket) == AVERROR(EAGAIN))
                {
                    av_log(mCodecContext, AV_LOG_ERROR, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
                    mPacketPending = true;
                    av_packet_move_ref(&mPendingPacket, &avPacket);
                }
            }

            // Packet has been set to decoder and we are done with it.
            av_packet_unref(&avPacket);
        }
    }
}

bool Decoder::ReachedEOF() const
{
    return mEofSerial == mPacketQueue->serial;
}
