//
// Clark Kromenaker
//
// Encapsulates logic to pull frames from video frame queue and convert to renderable format (texture).
//
// Because this system uses the engine's Texture class, it'll work for any graphics library it has implemented!
// In other words, this code should be platform-agnostic.
//
#pragma once
#include "Color32.h"

struct AVFrame;
struct Frame;
struct SwsContext;
class Texture;
struct VideoState;

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04

/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1

/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1

class VideoPlayback
{
public:
    ~VideoPlayback();

    void Update(VideoState* is);

    void SetStep(bool s) { mStep = s; }

    Texture* GetVideoTexture() { return mVideoTexture; }
    void RelenquishVideoTextureOwnership() { mOwnsVideoTexture = false; }

    void SetTransparentColor(const Color32& color);
    void ClearTransparentColor();

private:
    // ffmpeg provides a helper library for converting between formats.
    // For ease of use, all formats are converted to RGBA for display.
    SwsContext* mRGBAConvertContext = nullptr;

    // If true, playback is in "step" mode, where each frame is viewed one at a time.
    bool mStep = false;

    // Texture that current video frame will be written to.
    Texture* mVideoTexture = nullptr;

    // If true, this object owns the video texture, and needs to delete it when no longer needed.
    // If false, someone else has signaled that they will take care of deleting the video texture.
    bool mOwnsVideoTexture = true;

    // If set, this color in the video file will be considered transparent.
    bool mHasTransparentColor = false;
    Color32 mTransparentColor;

    bool UpdateVideoTexture(Frame* videoFrame);
    void UpdateSubtitles(VideoState* is);

    double CalculateFrameDuration(VideoState* is, Frame* vp, Frame* nextvp);
    double SyncVideo(VideoState* is, double delay);
};
