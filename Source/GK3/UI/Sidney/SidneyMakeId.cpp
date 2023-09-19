#include "SidneyMakeId.h"

#include "Actor.h"
#include "ActionManager.h"
#include "AssetManager.h"
#include "GameProgress.h"
#include "InventoryManager.h"
#include "SidneyButton.h"
#include "SidneyMenuBar.h"
#include "SidneyUtil.h"
#include "Scene.h"
#include "StringUtil.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UINineSlice.h"

void SidneyMakeId::Init(Actor* parent)
{
    // Add background. This will also be the root for this screen.
    mRoot = SidneyUtil::CreateBackground(parent);
    mRoot->SetName("MakeId");

    // Add main menu button.
    SidneyUtil::CreateMainMenuButton(mRoot, [&](){
        Hide();
    });

    // Add menu bar.
    mMenuBar.Init(mRoot, SidneyUtil::GetMakeIdLocalizer().GetText("ScreenName"), 120.0f);

    // The "Make ID" screen has a number of menu bar dropdowns.
    // Each dropdown corresponds to a job category, and each option is a specific job (e.g. Medical, Doctor).

    // To cut down on duplicate code, let's use a 2D vector corresponding to the job keywords used by the ID image resources.
    // This allows us to populate the dropdowns and all dropdown choices in a simple loop.
    std::vector<std::vector<std::string>> dropdownJobKeywords = {
        { "DOC", "CORONER", "BLOOD" },                      // Medical
        { "NYTIMES", "FREELANCE", "EMONTHLY", "SPORTSI" },  // Reporter
        { "ELEC", "PLUMB" },                                // Repair
        { "ENCY", "SHOES", "AUTO", "DIAPER" },              // Sales
        { "NOPD", "SECURITY" }                              // Police
    };
    for(size_t i = 0; i < dropdownJobKeywords.size(); ++i)
    {
        // Create main dropdown for this job category.
        std::string menuLocKey = "Menu" + std::to_string(i + 1) + "Name";
        mMenuBar.AddDropdown(SidneyUtil::GetMakeIdLocalizer().GetText(menuLocKey));

        // Create choices beneath this dropdown.
        for(size_t j = 0; j < dropdownJobKeywords[i].size(); ++j)
        {
            std::string choiceLocKey = "Menu" + std::to_string(i + 1) + "Item" + std::to_string(j + 1);
            std::string jobId = dropdownJobKeywords[i][j];

            // On click, we need to refresh the ID picture and the ID header text.
            mMenuBar.AddDropdownChoice(SidneyUtil::GetMakeIdLocalizer().GetText(choiceLocKey), [this, menuLocKey, choiceLocKey, jobId](){
                mJobId = jobId;
                RefreshId();
                RefreshIdHeader(menuLocKey, choiceLocKey);
            });
        }
    }

    // Create box/window for main ID making area.
    Actor* windowActor = new Actor(TransformType::RectTransform);
    windowActor->GetTransform()->SetParent(mRoot->GetTransform());
    {
        UINineSlice* border = windowActor->AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        border->GetRectTransform()->SetSizeDelta(426.0f, 282.0f);

        // Add one line for the box header.
        Actor* boxHeaderDividerActor = new Actor(TransformType::RectTransform);
        boxHeaderDividerActor->GetTransform()->SetParent(windowActor->GetTransform());

        UIImage* boxHeaderDividerImage = boxHeaderDividerActor->AddComponent<UIImage>();
        boxHeaderDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        boxHeaderDividerImage->GetRectTransform()->SetPivot(0.5f, 1.0f);
        boxHeaderDividerImage->GetRectTransform()->SetAnchorMin(0.0f, 1.0f);
        boxHeaderDividerImage->GetRectTransform()->SetAnchorMax(1.0f, 1.0f);
        boxHeaderDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -17.0f);
        boxHeaderDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        // Add header label.
        Actor* headerActor = new Actor(TransformType::RectTransform);
        headerActor->GetTransform()->SetParent(windowActor->GetTransform());

        mIdToPrintLabel = headerActor->AddComponent<UILabel>();
        mIdToPrintLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mIdToPrintLabel->SetText("MEDICAL ; DOCTOR");
        mIdToPrintLabel->SetHorizonalAlignment(HorizontalAlignment::Right);
        mIdToPrintLabel->SetVerticalAlignment(VerticalAlignment::Top);
        mIdToPrintLabel->SetMasked(true);

        mIdToPrintLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
        mIdToPrintLabel->GetRectTransform()->SetAnchoredPosition(-8.0f, -3.0f);
        mIdToPrintLabel->GetRectTransform()->SetSizeDeltaY(14.0f);
    }

    // Add character select.
    {
        Actor* charSelectWindowActor = new Actor(TransformType::RectTransform);
        charSelectWindowActor->GetTransform()->SetParent(windowActor->GetTransform());

        // Add framed area.
        {
            UINineSlice* border = charSelectWindowActor->AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
            border->GetRectTransform()->SetSizeDelta(76.0f, 192.0f);
            border->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            border->GetRectTransform()->SetAnchoredPosition(22.0f, 61.0f);
        }

        // Add header divider and "Select:" text.
        {
            Actor* dividerActor = new Actor(TransformType::RectTransform);
            dividerActor->GetTransform()->SetParent(charSelectWindowActor->GetTransform());

            UIImage* dividerImage = dividerActor->AddComponent<UIImage>();
            dividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);

            dividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
            dividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -17.0f);
            dividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

            Actor* headerActor = new Actor(TransformType::RectTransform);
            headerActor->GetTransform()->SetParent(charSelectWindowActor->GetTransform());

            UILabel* label = headerActor->AddComponent<UILabel>();
            label->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
            label->SetText(SidneyUtil::GetMakeIdLocalizer().GetText("Select"));
            label->SetHorizonalAlignment(HorizontalAlignment::Center);
            label->SetVerticalAlignment(VerticalAlignment::Top);
            label->SetMasked(true);

            label->GetRectTransform()->SetAnchor(AnchorPreset::Top);
            label->GetRectTransform()->SetAnchoredPosition(0.0f, -3.0f);
            label->GetRectTransform()->SetSizeDeltaY(14.0f);
        }

        // Add Gabriel button.
        {
            Actor* buttonActor = new Actor(TransformType::RectTransform);
            buttonActor->GetTransform()->SetParent(charSelectWindowActor->GetTransform());

            UIImage* buttonImage = buttonActor->AddComponent<UIImage>();
            buttonImage->SetTexture(gAssetManager.LoadTexture("GAB_MUGSHOT.BMP"), true);

            UIButton* button = buttonActor->AddComponent<UIButton>();
            button->SetPressCallback([this](UIButton* button){
                mCharacterId = "GAB";
                RefreshId();
            });

            button->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            button->GetRectTransform()->SetAnchoredPosition(4.0f, 91.0f);
        }

        // Add Grace button.
        {
            Actor* buttonActor = new Actor(TransformType::RectTransform);
            buttonActor->GetTransform()->SetParent(charSelectWindowActor->GetTransform());

            UIImage* buttonImage = buttonActor->AddComponent<UIImage>();
            buttonImage->SetTexture(gAssetManager.LoadTexture("GRA_MUGSHOT.BMP"), true);

            UIButton* button = buttonActor->AddComponent<UIButton>();
            button->SetPressCallback([this](UIButton* button){
                mCharacterId = "GRA";
                RefreshId();
            });

            button->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            button->GetRectTransform()->SetAnchoredPosition(4.0f, 6.0f);
        }
    }

    // Add ID preview window.
    {
        // The window with its border.
        Actor* previewWindowActor = new Actor(TransformType::RectTransform);
        previewWindowActor->GetTransform()->SetParent(windowActor->GetTransform());

        UINineSlice* border = previewWindowActor->AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        border->GetRectTransform()->SetSizeDelta(259.0f, 168.0f);
        border->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        border->GetRectTransform()->SetAnchoredPosition(140.0f, 62.0f);

        // The ID image.
        Actor* idActor = new Actor(TransformType::RectTransform);
        idActor->GetTransform()->SetParent(previewWindowActor->GetTransform());

        mIdToPrintImage = idActor->AddComponent<UIImage>();
        mIdToPrintImage->SetTexture(gAssetManager.LoadTexture("GAB_DOC.BMP"), true);
        mIdToPrintImage->GetRectTransform()->SetAnchor(AnchorPreset::Center);
        mIdToPrintImage->GetRectTransform()->SetAnchoredPosition(-0.5f, 0.0f); // small offset needed for pixel-perfect positioning
    }

    // Add the connector between the characters and the ID.
    {
        Actor* connectorActor = new Actor(TransformType::RectTransform);
        connectorActor->GetTransform()->SetParent(windowActor->GetTransform());

        UIImage* connectorImage = connectorActor->AddComponent<UIImage>();
        connectorImage->SetTexture(gAssetManager.LoadTexture("S_ID_BRACKET.BMP"), true);
        connectorImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        connectorImage->GetRectTransform()->SetAnchoredPosition(96.0f, 143.0f);
    }

    // Add button to print an ID to bottom right of box.
    {
        SidneyButton* button = new SidneyButton(windowActor);
        button->SetText(SidneyUtil::GetMakeIdLocalizer().GetText("Print"));
        button->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"));
        button->SetWidth(135.0f);

        button->GetRectTransform()->SetAnchor(AnchorPreset::BottomRight);
        button->GetRectTransform()->SetAnchoredPosition(-28.0f, 23.0f);

        button->SetPressCallback([this](){
            PrintId();
        });
    }

    Hide();
}

void SidneyMakeId::Show()
{
    mRoot->SetActive(true);
}

void SidneyMakeId::Hide()
{
    mRoot->SetActive(false);
}

void SidneyMakeId::OnUpdate(float deltaTime)
{
    if(!mRoot->IsActive()) { return; }
    mMenuBar.Update();
}

void SidneyMakeId::RefreshIdHeader(const std::string& category, const std::string& job)
{
    mIdToPrintLabel->SetText(SidneyUtil::GetMakeIdLocalizer().GetText(category) + " ; " +
                             SidneyUtil::GetMakeIdLocalizer().GetText(job));
}

void SidneyMakeId::RefreshId()
{
    mIdToPrintImage->SetTexture(gAssetManager.LoadTexture(mCharacterId + "_" + mJobId + ".BMP"));
}

void SidneyMakeId::PrintId()
{
    // Printing IDs is only allowed during Day 2, 2PM timeblock.
    if(gGameProgress.GetTimeblock() != Timeblock("202P"))
    {
        // Gabe/Grace say "I don't need to a fake ID".
        if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel"))
        {
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O8G5FVU1\", 1)");
        }
        else
        {
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O8G5FZ51\", 1)");
        }
    }
    else
    {
        // Day 2, 2PM: Gabe needs to print a fake ID to get into Chateau de Serras.
        // This is the ONLY time a fake ID is needed in the entire game!

        // First, if trying to print a Grace ID, Gabe will protest...because the picture is wrong.
        if(mCharacterId == "GRA")
        {
            // "Uh duh, what's wrong with this picture?"
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O8G5F961\", 1)");
        }
        else
        {
            // Gabe is pretty picky about which IDs he's willing to print.
            // He refuses to print most IDs, with only a subset actually being printable.
            if(mJobId == "DOC" || mJobId == "CORONER" || mJobId == "ELEC" || mJobId == "PLUMB" ||
               mJobId == "ENCY" || mJobId == "SHOES" || mJobId == "NOPD" || mJobId == "SECURITY")
            {
                // "I don't think this ID will get me into a vineyard..."
                gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O8G5F1K1\", 1)");
            }
            else if(mJobId == "EMONTHLY" || mJobId == "SPORTSI")
            {
                // "A reporter's good, but I don't think they'd be interested in this magazine."
                gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O8G5FNV1\", 1)");
            }
            else
            {
                // Gabe either enthusiastically says "That should work!" or "Might provoke an interesting response."
                const char* kThatShouldWorkVO = "wait StartDialogue(\"02O8G5F772\", 1)";
                const char* kInterestingResponseVO = "wait StartDialogue(\"02O8G5FQ21\", 1)";

                // Figure out inventory item for printed ID.
                // Also which VO to play when printing.
                std::string invItemName;
                std::string vo;
                if(mJobId == "NYTIMES")
                {
                    invItemName = "FAKE_ID_NYT_REP";
                    vo = kThatShouldWorkVO;
                }
                else if(mJobId == "FREELANCE")
                {
                    invItemName = "FAKE_ID_REPORTER";
                    vo = kThatShouldWorkVO;
                }
                else if(mJobId == "BLOOD")
                {
                    invItemName = "FAKE_ID_BLOODBANK";
                    vo = kInterestingResponseVO;
                }
                else if(mJobId == "AUTO")
                {
                    invItemName = "FAKE_ID_CAR";
                    vo = kInterestingResponseVO;
                }
                else if(mJobId == "DIAPER")
                {
                    invItemName = "FAKE_ID_DIAPERS";
                    vo = kInterestingResponseVO;
                }

                // If we already printed this, we don't print it again.
                if(gInventoryManager.HasInventoryItem(invItemName))
                {
                    // "I already have it."
                    gActionManager.ExecuteSheepAction("wait StartDialogue(\"0XF724XBL1\", 1)");
                }
                else
                {
                    // Play VO for printing this ID. 
                    gActionManager.ExecuteSheepAction(vo);

                    // Play a print SFX.
                    gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDPRINTID.WAV"));

                    // Grant the inventory item.
                    gInventoryManager.AddInventoryItem(invItemName);
                }
            }
        }
    }
}