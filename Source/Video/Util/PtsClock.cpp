#include "PtsClock.h"

#include <cmath>

extern "C"
{
    #include <libavutil/time.h>
}

/*static*/ double PtsClock::GetCurrentTimeSeconds()
{
    // av_gettime_relative returns microseconds, so divide by 1 million to get seconds.
    return av_gettime_relative() / 1000000.0;
}

PtsClock::PtsClock(int* queueSerial)
{
    // Track provided queue serial.
    // If null, track our own serial (i.e. always in sync).
    mPacketQueueSerial = queueSerial;
    if(mPacketQueueSerial == nullptr)
    {
        mPacketQueueSerial = &mSerial;
    }
    
    mLastUpdateTime = GetCurrentTimeSeconds();
}

void PtsClock::SetPtsToCurrentTime()
{
    // Force mPts equal to GetTime value.
    // Also updates mLastUpdateTime to current time.
    SetPts(GetTime(), mSerial);
}

void PtsClock::SetPts(double pts, int serial)
{
    SetPtsAtTime(pts, serial, GetCurrentTimeSeconds());
}

void PtsClock::SetPtsAtTime(double pts, int serial, double time)
{
    mPts = pts;
    mSerial = serial;
    mLastUpdateTime = time;
}

double PtsClock::GetTime()
{
    // If this clock's serial doesn't match the associated packet queue's serial, this clock is obsolete.
    // We should not return valid data until that is resolved.
    if(*mPacketQueueSerial != mSerial)
    {
        return std::numeric_limits<double>::quiet_NaN();
    }
    
    // When paused, clock time can just be equal to last pts.
    if(mPaused)
    {
        return mPts;
    }
    else
    {
        // Even if pts has not been updated recently, we want to take into account the passage of time.
        // "pts - lastUpdateTime" tells us the "drift" between the timestamp and update time.
        // This drift, plus current time, gives an accurate clock time.
        return (mPts - mLastUpdateTime) + GetCurrentTimeSeconds(); // - (time - last_updated) * (1.0 - speed);
    }
}

double PtsClock::GetSecondsSinceLastUpdate()
{
    return GetCurrentTimeSeconds() - mLastUpdateTime;
}

void PtsClock::SyncTo(PtsClock* other)
{
    double thisTime = GetTime();
    double otherTime = other->GetTime();
    if(!std::isnan(otherTime) && (std::isnan(thisTime) || ::fabs(thisTime - otherTime) > AV_NOSYNC_THRESHOLD))
    {
        SetPts(otherTime, other->mSerial);
    }
}
