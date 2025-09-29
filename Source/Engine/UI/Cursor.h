//
// Clark Kromenaker
//
// An asset that represents a mouse cursor.
//
#pragma once
#include "Asset.h"

#include <vector>

#include "Texture.h"
#include "Vector2.h"

struct SDL_Cursor;
class Texture;

class Cursor : public Asset
{
    TYPEINFO_SUB(Cursor, Asset);
public:
    Cursor(const std::string& name, AssetScope scope) : Asset(name, scope) { }
    ~Cursor() override;

    void Load(uint8_t* data, uint32_t dataLength);

    void Activate(bool animate = true);
    void Update(float deltaTime);

    bool IsAnimating() const { return mAnimate; }

private:
    //
    // ASSET DATA
    //
    // Each cursor has one texture.
    // In the case of animated cursors, the texture contains multiple frames.
    Texture* mTexture = nullptr;

    // The number of frames in the texture. Must be at least one.
    int mFrameCount = 1;

    // The position on the cursor that is considered "the tip" of the cursor.
    Vector2 mHotspot;

    // For animated cursors, the speed of the animation.
    float mFramesPerSecond = 0.0f;

    //
    // RUNTIME DATA
    //
    // The frames making up the cursor.
    // For animated cursors, there may be multiple entries.
    std::vector<SDL_Cursor*> mCursorFrames;

    // The scale factor the cursor frames were created at.
    // If the scale factor changes, the cursor frames must be regenerated.
    float mScaleFactor = 0.0f;

    // If true, cursor will animate.
    // Sometimes, we want to purposely disable this.
    bool mAnimate = true;

    // For animated cursors, the current frame index.
    float mFrameIndex = 0.0f;

    bool RecreateCursorFramesIfNeeded();
    void CreateCursorFrames(float scaleFactor);
    void FreeCursorFrames();
};