//
// Clark Kromenaker
//
// UI screen that shows an interface to get fingerprints from an item.
//
#pragma once
#include "Actor.h"

#include "LayerManager.h"
#include "RectTransform.h" // AnchorPreset
#include "StringUtil.h"

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

    // The exit button, for keyboard shortcut.
    UIButton* mExitButton = nullptr;

    // A big button over the right panel on this screen, which shows the object closeup.
    // This is mainly used to detect when dragging in this area.
    UIButton* mRightPanelButton = nullptr;

    // Image showing the object that is being dusted for prints.
    UIImage* mFingerprintObjectImage = nullptr;

    // Image of brush in the kit. Turned on and off when you select it.
    UIImage* mBrushImage = nullptr;

    // Represents one object that can be dusted for prints.
    struct FingerprintObject
    {
        // Texture corresponding to this object. Shows in right side of fingerprint UI.
        std::string textureName;

        // Position for the object texture in the right panel.
        AnchorPreset anchor = AnchorPreset::Left;
        Vector2 anchoredPosition;

        // Each object that can be dusted will have zero or more fingerprints to discover.
        // This is the list of each fingerprint instance on the object.
        struct Fingerprint
        {
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

            // In some cases, a fingerprint has more complex logic.
            std::function<void()> onCollectCustomLogicFunc = nullptr;
        };
        std::vector<Fingerprint> fingerprints;

        // Dialogue that plays if you dust the object and find no prints.
        std::string noPrintLicensePlate;

        // Dialogue to play when collecting each print.
        // These are separate from the Fingerprint struct because these are played *in order* regardless of which FP you collect first/middle/last/etc.
        std::vector<std::string> collectPrintLicensePlates;
    };
    std::string_map_ci<FingerprintObject> mObjects;

    // The object being actively dusted.
    FingerprintObject* mActiveObject = nullptr;

    // The prints that can be collected from the active object.
    // Usually, objects only have one print to collect, but occasionally there can be as many as three!
    static const int kMaxFingerprintImages = 3;
    struct PrintToCollect
    {
        // The image/button that corresponds to this print.
        UIImage* image = nullptr;
        UIButton* button = nullptr;

        // The current print alpha; this increases as you dust the print, until it is completely exposed.
        float alpha = 0.0f;

        // Whether the print has been collected or not.
        bool collected = false;
    };
    PrintToCollect mPrintsToCollect[kMaxFingerprintImages];

    // The number of prints we've collected for the current object.
    int mCollectedPrintCount = 0;

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
    void OnFingerprintPressed(int printToCollectIndex);
    void OnCollectedFingerprint(int printToCollectIndex);

    void GrantInvItemFlagAndScore(const FingerprintObject::Fingerprint& fp);

    void OnCollectWilkesDirtyGlass();
    void OnCollectBuchelliDirtyGlass();
};
