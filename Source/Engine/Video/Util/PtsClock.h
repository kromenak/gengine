//
// Based very heavily off of the "Clock" struct in ffmpeg's ffplay example.
//
// A clock that track the presentation timestamp (pts) for a video stream.
//
// During video playback, the various streams (video, audio) will not stay synched without some effort.
// Each stream keeps its own clock, which can be compared to ensure audio and video stay synced.
//
#pragma once
#include <limits>

/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

struct PtsClock
{
    static double GetCurrentTimeSeconds();

    PtsClock() = default;
    PtsClock(int* queueSerial);

    void SetPtsToCurrentTime();
    void SetPts(double newPts, int newSerial);
    void SetPtsAtTime(double newPts, int newSerial, double time);

    double GetPts() const { return mPts; }
    double GetTime();
    double GetSecondsSinceLastUpdate();

    void SetPaused(bool p) { mPaused = p; }
    bool IsPaused() const { return mPaused; }

    int GetSerial() const { return mSerial; }

    void SyncTo(PtsClock* other);

private:
    // The last "presentation time stamp" received by this clock. In seconds.
    // Each time a stream (video, audio, etc) presents to the user, this value is updated with the presented frame's PTS.
    double mPts = std::numeric_limits<double>::quiet_NaN();

    // Time (in seconds) at which this clock's PTS was last updated.
    double mLastUpdateTime = 0.0;

    // If true, clock is paused (calls to "Get" won't take passage of time into account).
    bool mPaused = false;

    // Serial of the stream this clock is tracking.
    int mSerial = -1;

    // Pointer to stream's packet queue's serial value.
    // If clock serial doesn't match this value, this clock's data is obsolete/invalid until updated.
    int* mPacketQueueSerial = nullptr;
};
