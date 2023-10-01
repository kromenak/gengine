#include "SidneyAnalyze.h"

#include "AssetManager.h"
//#include "AudioManager.h"
#include "InventoryManager.h"
#include "SidneyButton.h"
#include "SidneyFiles.h"
#include "SidneyUtil.h"
//#include "Texture.h"
#include "UIButton.h"
#include "UINineSlice.h"
#include "UIImage.h"
//#include "UILabel.h"

void SidneyAnalyze::Init(Actor* parent, SidneyFiles* sidneyFiles)
{
    mSidneyFiles = sidneyFiles;

    // Add background. This will also be the root for this screen.
    mRoot = SidneyUtil::CreateBackground(parent);
    mRoot->SetName("Email");

    // Add main menu button.
    SidneyUtil::CreateMainMenuButton(mRoot, [&](){
        Hide();
    });

    // Add menu bar.
    mMenuBar.Init(mRoot, SidneyUtil::GetAnalyzeLocalizer().GetText("ScreenName"), 120.0f);
    mMenuBar.SetFirstDropdownPosition(24.0f);
    mMenuBar.SetDropdownSpacing(26.0f);

    mMenuBar.AddDropdown(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu1Name"));
    mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("MenuItem1"), [this](){

        // Show the file selector.
        mSidneyFiles->Show([this](SidneyFile* selectedFile) {
            mSidneyFiles->Hide();

            mAnalyzeFile = selectedFile;

            // Show pre-analyze UI.
            mPreAnalyzeWindow->SetActive(true);
            mPreAnalyzeTitleLabel->SetText(mAnalyzeFile->GetDisplayName());
            mPreAnalyzeItemImage->SetTexture(mAnalyzeFile->GetIcon());

            // Make the analyze button clickable.
            mAnalyzeButton->GetButton()->SetCanInteract(true);
        });
    });

    mMenuBar.AddDropdown(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu2Name"));

    mMenuBar.AddDropdown(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Name"));

    mMenuBar.AddDropdown(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu4Name"));
    
    mMenuBar.SetDropdownEnabled(1, false);
    mMenuBar.SetDropdownEnabled(2, false);
    mMenuBar.SetDropdownEnabled(3, false);

    // Add analyze button.
    {
        SidneyButton* analyzeButton = SidneyUtil::CreateBigButton(mRoot);
        analyzeButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("MenuItem2"));
        float labelWidth = analyzeButton->GetLabel()->GetTextWidth() + 12.0f;
        analyzeButton->SetWidth(labelWidth);

        analyzeButton->GetRectTransform()->SetPivot(1.0f, 0.0f); // Bottom-Right
        analyzeButton->GetRectTransform()->SetAnchor(1.0f, 0.0f); // Bottom-Right
        analyzeButton->GetRectTransform()->SetAnchoredPosition(-113.0f, 10.0f); // offset from bottom-right

        analyzeButton->GetButton()->SetCanInteract(false);

        mAnalyzeButton = analyzeButton;
        mAnalyzeButton->SetPressCallback([this](){
            AnalyzeFile();
        });
    }

    // Add pre-analyze window.
    // Create box/window for main ID making area.
    mPreAnalyzeWindow = new Actor(TransformType::RectTransform);
    mPreAnalyzeWindow->GetTransform()->SetParent(mRoot->GetTransform());
    {
        UINineSlice* border = mPreAnalyzeWindow->AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        border->GetRectTransform()->SetSizeDelta(153.0f, 167.0f);

        border->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        border->GetRectTransform()->SetAnchoredPosition(40.0f, 203.0f);

        // Add one line for the box header.
        Actor* boxHeaderDividerActor = new Actor(TransformType::RectTransform);
        boxHeaderDividerActor->GetTransform()->SetParent(mPreAnalyzeWindow->GetTransform());

        UIImage* boxHeaderDividerImage = boxHeaderDividerActor->AddComponent<UIImage>();
        boxHeaderDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        boxHeaderDividerImage->GetRectTransform()->SetPivot(0.5f, 1.0f);
        boxHeaderDividerImage->GetRectTransform()->SetAnchorMin(0.0f, 1.0f);
        boxHeaderDividerImage->GetRectTransform()->SetAnchorMax(1.0f, 1.0f);
        boxHeaderDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -17.0f);
        boxHeaderDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        // Add header label.
        Actor* headerActor = new Actor(TransformType::RectTransform);
        headerActor->GetTransform()->SetParent(mPreAnalyzeWindow->GetTransform());

        mPreAnalyzeTitleLabel = headerActor->AddComponent<UILabel>();
        mPreAnalyzeTitleLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mPreAnalyzeTitleLabel->SetText("Map");
        mPreAnalyzeTitleLabel->SetHorizonalAlignment(HorizontalAlignment::Right);
        mPreAnalyzeTitleLabel->SetVerticalAlignment(VerticalAlignment::Top);
        mPreAnalyzeTitleLabel->SetMasked(true);

        mPreAnalyzeTitleLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
        mPreAnalyzeTitleLabel->GetRectTransform()->SetAnchoredPosition(-8.0f, -3.0f);
        mPreAnalyzeTitleLabel->GetRectTransform()->SetSizeDeltaY(14.0f);

        // Add item image.
        Actor* imageActor = new Actor(TransformType::RectTransform);
        imageActor->GetTransform()->SetParent(mPreAnalyzeWindow->GetTransform());

        mPreAnalyzeItemImage = imageActor->AddComponent<UIImage>();
        mPreAnalyzeItemImage->SetTexture(gAssetManager.LoadTexture("MAP9.BMP"), true);
        mPreAnalyzeItemImage->GetRectTransform()->SetAnchoredPosition(0.0f, -8.0f);

        // Hide pre-analyze window by default.
        mPreAnalyzeWindow->SetActive(false);
    }

    // Hide by default.
    Hide();
}

void SidneyAnalyze::Show()
{
    mRoot->SetActive(true);
}

void SidneyAnalyze::Hide()
{
    mRoot->SetActive(false);
}

void SidneyAnalyze::OnUpdate(float deltaTime)
{
    if(!mRoot->IsActive()) { return; }
    mMenuBar.Update();
}

void SidneyAnalyze::AnalyzeFile()
{
    if(mAnalyzeFile == nullptr) { return; }
    printf("Analyze %s\n", mAnalyzeFile->GetDisplayName().c_str());
}