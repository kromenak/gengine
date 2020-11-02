//
// VideoPlaybackGL.h
//
// Clark Kromenaker
//
// Description
//
#pragma once
#include "Material.h"

struct AVFrame;
struct Frame;
struct SwsContext;
struct VideoState;
class Texture;

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1

class VideoPlaybackGL
{
public:
    ~VideoPlaybackGL();
    
    void Update(VideoState* is);
    
    void SetStep(bool s) { step = s; }
    
    Texture* GetVideoTexture() { return mVideoTexture; }
    
private:
    SwsContext* mRBGAConvertContext = nullptr;
    //SwsContext* sub_convert_ctx = nullptr;
    
    // If true, playback is in "step" mode, where each frame is viewed one at a time.
    bool step = false;
    
    Texture* mVideoTexture = nullptr;
    
    bool UpdateVideoTexture(Frame* videoFrame);
    
    void UpdateSubtitles(VideoState* is);
    
    double CalculateFrameDuration(VideoState* is, Frame* vp, Frame* nextvp);
    double SyncVideo(VideoState* is, double delay);
};
