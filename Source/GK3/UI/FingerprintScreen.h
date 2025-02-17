//
// Clark Kromenaker
//
// UI screen that shows an interface to get fingerprints from an item.
//
#pragma once
#include "Actor.h"

#include "LayerManager.h"
#include "StringUtil.h"
#include "Timeblock.h"

class Cursor;
class UIButton;
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

    // Cursors used at different points on this screen.
    Cursor* mBrushCursor = nullptr;
    Cursor* mDustedBrushCursor = nullptr;
    Cursor* mTapeCursor = nullptr;
    Cursor* mUsedTapeCursor = nullptr;

    // A big button over the right panel on this screen, which shows the object closeup.
    // This is mainly used to detect when dragging in this area.
    UIButton* mRightPanelButton = nullptr;

    // Image showing the object that is being dusted for prints.
    UIImage* mFingerprintObjectImage = nullptr;

    // Image of brush in the kit. Turned on and off when you select it.
    UIImage* mBrushImage = nullptr;

    static const int kMaxFingerprintImages = 3;
    UIImage* mFingerprintImages[kMaxFingerprintImages] = { 0 };
    UIButton* mFingerprintButtons[kMaxFingerprintImages] = { 0 };
    float mFingerprintAlpha[kMaxFingerprintImages] = { 0.0f };

    // Represents one object that can be dusted for prints.
    struct FingerprintObject
    {
        // Texture corresponding to this object. Shows in right side of fingerprint UI.
        std::string textureName;

        // Each object that can be dusted will have zero or more fingerprints to discover.
        // This is the list of each fingerprint instance on the object.
        struct Fingerprint
        {
            // If set, this fingerprint only appears on this timeblock. The default value here means "not set, ignore".
            Timeblock timeblock = Timeblock(1, 0);

            // The name of the texture that corresponds to this fingerprint.
            std::string textureName;

            // The position of the fingerprint on the object, from the bottom-left corner of the UI panel.
            Vector2 position;

            // The name of the inventory item granted upon collecting this print.
            std::string invItemName;

            // A score name to trigger when collecting this print.
            std::string scoreName;

            // A flag to set upon collecting this print.
            // Rarely, Grace uses a different flag than Gabe. If not set, she uses the regular one.
            std::string flagName;
            std::string flagNameGrace;

            // Dialogue that plays when uncovering and collecting this print.
            std::string uncoverPrintLicensePlate;
            std::string collectPrintLicensePlate;
        };
        std::vector<Fingerprint> fingerprints;

        // Dialogue that plays if you dust the object and find no prints.
        std::string noPrintLicensePlate;
    };
    std::string_map_ci<FingerprintObject> mObjects;

    // The object being actively dusted.
    FingerprintObject* mActiveObject = nullptr;

    // The cursor on this screen is pretty complex and changes based on whether you're holding the brush, holding tape, etc.
    enum class CursorState
    {
        Normal,         // Normal everyday cursor.
        Brush,          // Holding a clean brush.
        DustedBrush,    // Holding a brush with dust applied.
        Tape,           // Holding clean piece of tape.
        TapeWithPrint   // Holding tape with print.
    };
    CursorState mCursorState = CursorState::Normal;

    // When cursor state is "TapeWithPrint", which print index is being held?
    int mTapePrintIndex = -1;

    // When dusting, the total distance we've dragged the brush across the object.
    // Mainly used when an object has no prints - after dragging X distance, "no print" dialogue plays.
    float mDistanceDusted = 0.0f;
    
    void PickUpBrush();
    void PutDownBrush();

    void OnBackgroundButtonPressed();
    void OnBrushButtonPressed();
    void OnDustButtonPressed();
    void OnTapeButtonPressed();
    void OnClothButtonPressed();

    void OnRightPanelPressed();
    void OnFingerprintPressed(int index);
};
