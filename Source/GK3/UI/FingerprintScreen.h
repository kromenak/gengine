//
// Clark Kromenaker
//
// UI screen that shows an interface to get fingerprints from an item.
//
#pragma once
#include "Actor.h"

#include "LayerManager.h"
#include "StringUtil.h"

class UIImage;

class FingerprintScreen : public Actor
{
public:
    FingerprintScreen();
	
	void Show(const std::string& nounName);
	void Hide();
    bool IsShowing() const;

protected:
    void OnUpdate(float deltaTime) override;
	
private:
    // This screen's layer.
    Layer mLayer;

    // Image showing the object that is being dusted for prints.
    UIImage* mFingerprintObjectImage = nullptr;

    // Image of brush in the kit. Turned on and off when you select it.
    UIImage* mBrushImage = nullptr;

    // Represents one object that can be dusted for prints.
    struct FingerprintObject
    {
        // Texture corresponding to this object.
        std::string textureName;

        struct Fingerprint
        {
            std::string textureName;
            Vector2 position;
        };
        std::vector<Fingerprint> fingerprints;
    };
    std::string_map_ci<FingerprintObject> mObjects;

    enum class CursorState
    {
        Normal,
        CleanBrush,
        InkedBrush,
        CleanTape,
        InkedTape
    };
    CursorState mCursorState = CursorState::Normal;
    
    void PickUpBrush();
    void PutDownBrush();

    void OnBrushButtonPressed();
    void OnInkButtonPressed();
};
