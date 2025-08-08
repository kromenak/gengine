#include "Tooltip.h"

#include "AssetManager.h"
#include "IniParser.h"
#include "InputManager.h"
#include "Localizer.h"
#include "StringUtil.h"
#include "TextAsset.h"
#include "UICanvas.h"
#include "UILabel.h"
#include "UINineSlice.h"
#include "UIUtil.h"
#include "Window.h"

Tooltip* Tooltip::Get()
{
    //TODO: We could support different types of tooltips by passing in the asset name here.
    //TODO: However...the game only ever supported ONE type of tooltip. So keep it simple for now!
    //static std::string_map_ci<Tooltip*> sTooltips;
    //auto it = sTooltips.find(assetName);
    static Tooltip* sTooltip = nullptr;

    // If null, create it.
    if(sTooltip == nullptr)
    {
        sTooltip = new Tooltip();
        sTooltip->SetIsDestroyOnLoad(false);
    }
    return sTooltip;
}

Tooltip::Tooltip() : Actor("Tooltip", TransformType::RectTransform)
{
    UI::AddCanvas(this, 101);

    Color32 backgroundColor;
    Color32 borderColor;
    {
        TextAsset* textAsset = gAssetManager.LoadText("DEFAULT.TIP", AssetScope::Manual);
        IniParser parser(textAsset->GetText(), textAsset->GetTextLength());
        parser.SetMultipleKeyValuePairsPerLine(false);
        IniSection section;
        if(parser.ReadNextSection(section))
        {
            for(auto& line : section.lines)
            {
                if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "Font Name"))
                {
                    mFont = gAssetManager.LoadFont(line.entries[0].value);
                }
                else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "Debug Font Name"))
                {
                    mDebugFont = gAssetManager.LoadFont(line.entries[0].value);
                }
                else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "Show Delay"))
                {
                    // Value is in milliseconds, we want it in seconds.
                    mShowDelay = line.entries[0].GetValueAsInt() / 1000.0f;
                }
                else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "Quick Show Delay"))
                {
                    // Value is in milliseconds, we want it in seconds.
                    mQuickShowDelay = line.entries[0].GetValueAsInt() / 1000.0f;
                }
                else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "Background Color"))
                {
                    backgroundColor = line.entries[0].GetValueAsColor32();
                }
                else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "Border Color"))
                {
                    borderColor = line.entries[0].GetValueAsColor32();
                }
                else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "Text Border"))
                {
                    mTextPadding = line.entries[0].GetValueAsVector4();
                }
                //TODO: Background Bitmap, but never used in original game...
            }
        }
        delete textAsset;
    }

    // Create nine slice params for border/center colors.
    UINineSliceParams params;
    params.bottomColor = params.bottomLeftColor = params.leftColor = params.topLeftColor =
        params.topColor = params.topRightColor = params.rightColor = params.bottomRightColor = borderColor;
    params.centerColor = backgroundColor;

    // Create border and window for the tooltip.
    UINineSlice* window = UI::CreateWidgetActor<UINineSlice>("Box", this, params);
    mRoot = window->GetRectTransform();
    mRoot->SetPixelPerfect(true);
    mRoot->SetAnchor(AnchorPreset::BottomLeft);

    // Create text label inside the window.
    mLabel = UI::CreateWidgetActor<UILabel>("Label", window);
    mLabel->SetFont(mFont);
    mLabel->SetHorizonalAlignment(HorizontalAlignment::Left);
    mLabel->SetHorizontalOverflow(HorizontalOverflow::Overflow);
    mLabel->SetVerticalAlignment(VerticalAlignment::Bottom);
    mLabel->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    mLabel->GetRectTransform()->SetPixelPerfect(true);

    // Hidden by default.
    mRoot->GetOwner()->SetActive(false);
}

void Tooltip::Show(const std::string& text, UIWidget* hoverWidget, bool debug)
{
    // Set font based on debug or not.
    mLabel->SetFont(debug ? mDebugFont : mFont);

    // Set text. Tooltip may be localized - if so, these localizations are defined in the global loc file.
    mLabel->SetText(gLocalizer.GetText(text));

    // Based on the text width/height, figure out how big the tooltip window should be, taking padding into account.
    float labelWidth = mLabel->GetTextWidth();
    float labelHeight = mLabel->GetTextHeight();
    mRoot->SetSizeDelta(labelWidth + mTextPadding.x + mTextPadding.z,
                        labelHeight + mTextPadding.y + mTextPadding.w);

    // Fit the label tightly around its own text.
    mLabel->GetRectTransform()->SetSizeDelta(labelWidth, labelHeight);

    // Position the label correctly within the frame.
    mLabel->GetRectTransform()->SetAnchoredPosition(mTextPadding.x, mTextPadding.w);

    // Rather than show the tooltip right away, we apply a short delay.
    // The length of this delay depends on how long the tooltip has been hidden since it was last shown.
    mShowDelayTimer = mHiddenTimer < 0.5f ? mQuickShowDelay : mShowDelay;

    // Save hover widget so we can track if the widget stays hovered or not.
    mHoverWidget = hoverWidget;
}

void Tooltip::Hide()
{
    // Hide the tooltip root.
    mRoot->GetOwner()->SetActive(false);

    // Clear show delay timer to stop errantly showing after timer expires.
    mShowDelayTimer = 0.0f;

    // Clear hover widget.
    mHoverWidget = nullptr;
}

void Tooltip::OnUpdate(float deltaTime)
{
    // If show delay timer is active, decrement and show when the timer is up.
    if(mShowDelayTimer > 0.0f)
    {
        mShowDelayTimer -= deltaTime;
        if(mShowDelayTimer <= 0.0f)
        {
            // Position the tooltip relative to the mouse cursor.
            // First, get mouse position in tooltip's local space.
            Vector2 tooltipPos = GetTransform()->GetWorldToLocalMatrix().TransformPoint(gInputManager.GetMousePosition());

            // Position tooltip about 40 units below the mouse pointer.
            // BUT if too close to bottom of the screen, move it above the mouse pointer instead!
            float newY = tooltipPos.y - 40.0f;
            if(newY < 10.0f)
            {
                newY = tooltipPos.y + 10.0f;
            }
            tooltipPos.y = newY;
            mRoot->SetAnchoredPosition(tooltipPos);

            // Also make sure it stays on-screen horizontally.
            mRoot->MoveInsideRect(static_cast<RectTransform*>(GetTransform())->GetWorldRect());

            // Show the tooltip root.
            mRoot->GetOwner()->SetActive(true);

            // Clear hidden timer, since it was just shown.
            mHiddenTimer = 0.0f;
        }
    }

    // When hidden, track time so we know whether to do a quick show next time.
    if(!mRoot->GetOwner()->IsActive())
    {
        mHiddenTimer += deltaTime;
    }

    // If there's an associated hover widget, stop showing the tooltip if the widget is no longer hovered.
    // This is necessary so that tooltips stop hiding when hover stops, even if UICanvas isn't updating due to loading or an action playing.
    if(mHoverWidget != nullptr)
    {
        // An obvious case is when the hover widget's rect no longer contains the mouse pointer.
        // However, when mouse is locked, the mouse technically doesn't move, so is still in the rect...BUT the pointer is gone, so consider that "no longer hovered."
        if(!mHoverWidget->IsActiveAndEnabled() ||
           !mHoverWidget->GetRectTransform()->GetWorldRect().Contains(gInputManager.GetMousePosition()) ||
           gInputManager.IsMouseLocked())
        {
            Hide();
        }
    }
}