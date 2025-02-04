#include "SidneyEmail.h"

#include "ActionManager.h"
#include "Actor.h"
#include "AssetManager.h"
#include "AudioManager.h"
#include "Font.h"
#include "GameProgress.h"
#include "IniParser.h"
#include "SidneyButton.h"
#include "SidneyUtil.h"
#include "TextAsset.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UINineSlice.h"
#include "UIScrollRect.h"
#include "UIUtil.h"

void SidneyEmail::Init(Actor* parent)
{
    // Add background. This will also be the root for this screen.
    mRoot = SidneyUtil::CreateBackground(parent);
    mRoot->SetName("Email");

    // Add main menu button.
    SidneyUtil::CreateMainMenuButton(mRoot, [&](){
        Hide();
    });

    // Add menu bar.
    mMenuBar.Init(mRoot, SidneyUtil::GetEmailLocalizer().GetText("ScreenName"), 84.0f);

    // Create email list window.
    {
        UINineSlice* emailListWindow = UIUtil::NewUIActorWithWidget<UINineSlice>(mRoot, SidneyUtil::GetGrayBoxParams(Color32::Black));
        emailListWindow->GetRectTransform()->SetSizeDelta(250.0f, 170.0f);
        mEmailListWindow = emailListWindow->GetOwner();

        // Add one line for the box header.
        UIImage* headerDividerImage = UIUtil::NewUIActorWithWidget<UIImage>(emailListWindow->GetOwner());
        headerDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        headerDividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        headerDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -20.0f);
        headerDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        // Add header text.
        UILabel* headerLabel = UIUtil::NewUIActorWithWidget<UILabel>(emailListWindow->GetOwner());
        headerLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_18.FON"));
        headerLabel->SetText(SidneyUtil::GetEmailLocalizer().GetText("EMailList"));
        headerLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        headerLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        headerLabel->GetRectTransform()->SetSizeDelta(0.0f, 20.0f);

        // Add close button.
        SidneyUtil::CreateCloseWindowButton(emailListWindow->GetOwner(), [this](){
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDEXIT.WAV"));
            Hide();
        });
    }

    // Create single email view.
    {
        // Create window around entire area.
        UINineSlice* emailWindow = UIUtil::NewUIActorWithWidget<UINineSlice>(mRoot, SidneyUtil::GetGrayBoxParams(Color32::Black));
        emailWindow->GetRectTransform()->SetSizeDelta(531.0f, 358.0f);
        mEmailWindow = emailWindow->GetOwner();

        // Create header divider line.
        UIImage* headerDividerImage = UIUtil::NewUIActorWithWidget<UIImage>(emailWindow->GetOwner());
        headerDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        headerDividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        headerDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -100.0f);
        headerDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        // Create footer divider line.
        UIImage* footerDividerImage = UIUtil::NewUIActorWithWidget<UIImage>(emailWindow->GetOwner());
        footerDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        footerDividerImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomStretch);
        footerDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, 29.0f);
        footerDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        Font* font = gAssetManager.LoadFont("SID_TEXT_14.FON");

        // In the header, create all the data fields.
        {
            // From
            {
                UILabel* fromLabel = UIUtil::NewUIActorWithWidget<UILabel>(emailWindow->GetOwner());
                fromLabel->SetFont(font);
                fromLabel->SetText(SidneyUtil::GetEmailLocalizer().GetText("From"));
                fromLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                fromLabel->GetRectTransform()->SetAnchoredPosition(10.0f, -11.0f);
                fromLabel->GetRectTransform()->SetSizeDelta(fromLabel->GetTextWidth(), font->GetGlyphHeight());

                UINineSlice* fromBorder = UIUtil::NewUIActorWithWidget<UINineSlice>(emailWindow->GetOwner(), SidneyUtil::GetGrayBoxParams(Color32::Black));
                fromBorder->GetRectTransform()->SetSizeDelta(202.0f, 18.0f);
                fromBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                fromBorder->GetRectTransform()->SetAnchoredPosition(90.0f, -9.0f);

                mFromLabel = UIUtil::NewUIActorWithWidget<UILabel>(fromBorder->GetOwner());
                mFromLabel->SetFont(font);
                mFromLabel->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
                mFromLabel->GetRectTransform()->SetSizeDelta(-8.0f, -3.0f);
            }

            // To
            {
                UILabel* toLabel = UIUtil::NewUIActorWithWidget<UILabel>(emailWindow->GetOwner());
                toLabel->SetFont(font);
                toLabel->SetText(SidneyUtil::GetEmailLocalizer().GetText("To"));
                toLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                toLabel->GetRectTransform()->SetAnchoredPosition(10.0f, -30.0f);
                toLabel->GetRectTransform()->SetSizeDelta(toLabel->GetTextWidth(), font->GetGlyphHeight());

                UINineSlice* toBorder = UIUtil::NewUIActorWithWidget<UINineSlice>(emailWindow->GetOwner(), SidneyUtil::GetGrayBoxParams(Color32::Black));
                toBorder->GetRectTransform()->SetSizeDelta(202.0f, 18.0f);
                toBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                toBorder->GetRectTransform()->SetAnchoredPosition(90.0f, -28.0f);

                mToLabel = UIUtil::NewUIActorWithWidget<UILabel>(toBorder->GetOwner());
                mToLabel->SetFont(font);
                mToLabel->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
                mToLabel->GetRectTransform()->SetSizeDelta(-8.0f, -3.0f);
            }

            // CC
            {
                UILabel* ccLabel = UIUtil::NewUIActorWithWidget<UILabel>(emailWindow->GetOwner());
                ccLabel->SetFont(font);
                ccLabel->SetText(SidneyUtil::GetEmailLocalizer().GetText("CC"));
                ccLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                ccLabel->GetRectTransform()->SetAnchoredPosition(10.0f, -49.0f);
                ccLabel->GetRectTransform()->SetSizeDelta(ccLabel->GetTextWidth(), font->GetGlyphHeight());

                UINineSlice* ccBorder = UIUtil::NewUIActorWithWidget<UINineSlice>(emailWindow->GetOwner(), SidneyUtil::GetGrayBoxParams(Color32::Black));
                ccBorder->GetRectTransform()->SetSizeDelta(202.0f, 18.0f);
                ccBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                ccBorder->GetRectTransform()->SetAnchoredPosition(90.0f, -47.0f);

                mCCLabel = UIUtil::NewUIActorWithWidget<UILabel>(ccBorder->GetOwner());
                mCCLabel->SetFont(font);
                mCCLabel->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
                mCCLabel->GetRectTransform()->SetSizeDelta(-8.0f, -3.0f);
            }

            // Subject
            {
                UILabel* subjectLabel = UIUtil::NewUIActorWithWidget<UILabel>(emailWindow->GetOwner());
                subjectLabel->SetFont(font);
                subjectLabel->SetText(SidneyUtil::GetEmailLocalizer().GetText("Subject"));
                subjectLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                subjectLabel->GetRectTransform()->SetAnchoredPosition(10.0f, -78.0f);
                subjectLabel->GetRectTransform()->SetSizeDelta(subjectLabel->GetTextWidth(), font->GetGlyphHeight());

                UINineSlice* subjectBorder = UIUtil::NewUIActorWithWidget<UINineSlice>(emailWindow->GetOwner(), SidneyUtil::GetGrayBoxParams(Color32::Black));
                subjectBorder->GetRectTransform()->SetSizeDelta(202.0f, 18.0f);
                subjectBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                subjectBorder->GetRectTransform()->SetAnchoredPosition(90.0f, -76.0f);

                mSubjectLabel = UIUtil::NewUIActorWithWidget<UILabel>(subjectBorder->GetOwner());
                mSubjectLabel->SetFont(font);
                mSubjectLabel->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
                mSubjectLabel->GetRectTransform()->SetSizeDelta(-8.0f, -3.0f);
            }

            // Date
            {
                UINineSlice* dateBorder = UIUtil::NewUIActorWithWidget<UINineSlice>(emailWindow->GetOwner(), SidneyUtil::GetGrayBoxParams(Color32::Black));
                dateBorder->GetRectTransform()->SetSizeDelta(136.0f, 18.0f);
                dateBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                dateBorder->GetRectTransform()->SetAnchoredPosition(303.0f, -28.0f);

                mDateLabel = UIUtil::NewUIActorWithWidget<UILabel>(dateBorder->GetOwner());
                mDateLabel->SetFont(font);
                mDateLabel->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
                mDateLabel->GetRectTransform()->SetSizeDelta(-8.0f, -3.0f);
            }

            // Schattenjaeger logo.
            UIImage* logoImage = UIUtil::NewUIActorWithWidget<UIImage>(emailWindow->GetOwner());
            logoImage->SetTexture(gAssetManager.LoadTexture("S_SCHAT_LOGO.BMP"), true);
            logoImage->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
            logoImage->GetRectTransform()->SetAnchoredPosition(-7.0f, -13.0f);
        }

        // Create the body area.
        {
            // Create body canvas, for masking.
            UICanvas* bodyCanvas = UIUtil::NewUIActorWithCanvas(emailWindow->GetOwner(), -1);
            bodyCanvas->SetMasked(true);
            bodyCanvas->GetRectTransform()->SetAnchor(AnchorPreset::Top);
            bodyCanvas->GetRectTransform()->SetAnchoredPosition(0.0f, -100.0f);
            bodyCanvas->GetRectTransform()->SetSizeDelta(527.0f, 227.0f);

            // Create body scroll rect.
            mBodyScrollRect = new UIScrollRect(bodyCanvas->GetOwner());
            mBodyScrollRect->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
            mBodyScrollRect->GetRectTransform()->SetSizeDelta(0.0f, 0.0f);
            mBodyScrollRect->SetScrollbarWidth(5.0f);
        }

        // In the footer, create all the buttons.
        {
            SidneyButton* nextButton = new SidneyButton(emailWindow->GetOwner());
            nextButton->SetText(SidneyUtil::GetEmailLocalizer().GetText("Next"));
            nextButton->SetWidth(66.0f);
            nextButton->SetHeight(13.0f);
            nextButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            nextButton->GetRectTransform()->SetAnchoredPosition(63.0f, 8.0f);
            nextButton->SetPressCallback([this](){
                OnNextEmailButtonPressed();
            });
            mNextButton = nextButton;

            SidneyButton* prevButton = new SidneyButton(emailWindow->GetOwner());
            prevButton->SetText(SidneyUtil::GetEmailLocalizer().GetText("Previous"));
            prevButton->SetWidth(66.0f);
            prevButton->SetHeight(13.0f);
            prevButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            prevButton->GetRectTransform()->SetAnchoredPosition(141.0f, 8.0f);
            prevButton->SetPressCallback([this](){
                OnPrevEmailButtonPressed();
            });
            mPrevButton = prevButton;

            SidneyButton* replyButton = new SidneyButton(emailWindow->GetOwner());
            replyButton->SetText(SidneyUtil::GetEmailLocalizer().GetText("Reply"));
            replyButton->SetWidth(66.0f);
            replyButton->SetHeight(13.0f);
            replyButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            replyButton->GetRectTransform()->SetAnchoredPosition(219.0f, 8.0f);
            replyButton->SetPressCallback([this](){
                OnReplyButtonPressed();
            });

            SidneyButton* composeButton = new SidneyButton(emailWindow->GetOwner());
            composeButton->SetText(SidneyUtil::GetEmailLocalizer().GetText("Compose"));
            composeButton->SetWidth(66.0f);
            composeButton->SetHeight(13.0f);
            composeButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            composeButton->GetRectTransform()->SetAnchoredPosition(297.0f, 8.0f);
            composeButton->SetPressCallback([this](){
                OnComposeButtonPressed();
            });

            SidneyButton* printButton = new SidneyButton(emailWindow->GetOwner());
            printButton->SetText(SidneyUtil::GetEmailLocalizer().GetText("Print"));
            printButton->SetWidth(66.0f);
            printButton->SetHeight(13.0f);
            printButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            printButton->GetRectTransform()->SetAnchoredPosition(375.0f, 8.0f);
            printButton->SetPressCallback([this](){
                OnPrintButtonPressed();
            });

            SidneyButton* closeButton = new SidneyButton(emailWindow->GetOwner());
            closeButton->SetText(SidneyUtil::GetEmailLocalizer().GetText("Exit"));
            closeButton->SetWidth(50.0f);
            closeButton->SetHeight(13.0f);
            closeButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            closeButton->GetRectTransform()->SetAnchoredPosition(469.0f, 8.0f);
            closeButton->SetPressCallback([this](){
                OnCloseButtonPressed();
            });
        }
    }

    // Read in email data.
    {
        TextAsset* textFile = gAssetManager.LoadText(Localizer::GetLanguagePrefix() + "SIDNEYEMAIL.TXT");
        IniParser parser(textFile->GetText(), textFile->GetTextLength());
        parser.SetMultipleKeyValuePairsPerLine(false);

        // Each section corresponds to a specific email.
        // The data in the section relates to that email (from, to, subject, body, etc).
        IniSection section;
        while(parser.ReadNextSection(section))
        {
            // One section lists the emails in order, but we don't really need to parse this.
            if(section.name.empty() || StringUtil::EqualsIgnoreCase(section.name, "Email Files"))
            {
                continue;
            }

            Email& email = mAllEmails[section.name];
            for(auto& line : section.lines)
            {
                if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "From"))
                {
                    email.from = line.entries[0].value;
                }
                else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "To"))
                {
                    email.to = line.entries[0].value;
                }
                else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "CC"))
                {
                    email.cc = line.entries[0].value;
                }
                else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "Date"))
                {
                    email.date = line.entries[0].value;
                }
                else if(StringUtil::EqualsIgnoreCase(line.entries[0].key, "Subject"))
                {
                    email.subject = line.entries[0].value;
                }
                else if(StringUtil::StartsWithIgnoreCase(line.entries[0].key, "Body"))
                {
                    // The key contains a number, which is the index of the piece of body text.
                    int bodyIndex = StringUtil::ToInt(line.entries[0].key.substr(4)) - 1;

                    // Make sure we have enough entries to get up to this index.
                    while(email.body.size() <= bodyIndex)
                    {
                        email.body.emplace_back();
                    }

                    // Fill in the text. Skip <space> entries, as those are just blank lines.
                    if(!StringUtil::EqualsIgnoreCase(line.entries[0].value, "<space>"))
                    {
                        email.body[bodyIndex] = line.entries[0].value;
                    }
                }
            }
        }
    }

    // At game start, you've already got three emails.
    ReceiveEmail("Email1");
    ReceiveEmail("Email2");
    ReceiveEmail("Email3");

    // Hide by default.
    Hide();
}

void SidneyEmail::Show()
{
    // If Easter Egg system is enabled, you get the Easter Egg mail.
    if(gGameProgress.GetFlag("Egg"))
    {
        ReceiveEmail("EMail6");
    }

    // Show entire page.
    mRoot->SetActive(true);

    // Always default to the list view.
    ShowEmailList();
}

void SidneyEmail::Hide()
{
    mRoot->SetActive(false);
}

void SidneyEmail::ReceiveEmail(const std::string& emailId)
{
    // Add it, but make sure we don't add a dupe.
    auto it = std::find(mReceivedEmails.begin(), mReceivedEmails.end(), emailId);
    if(it == mReceivedEmails.end())
    {
        mReceivedEmails.push_back(emailId);
    }  
}

void SidneyEmail::ShowEmailList()
{
    // Show the list, hide the single item view.
    mEmailWindow->SetActive(false);
    mEmailListWindow->SetActive(true);

    // Two fonts are used, depending on whether an email is read or not.
    Font* greenFont = gAssetManager.LoadFont("SID_TEXT_14_GRN.FON");
    Font* yellowFont = gAssetManager.LoadFont("SID_TEXT_14.FON");

    // Show a list item for each email we have received.
    Vector2 listItemPos(8.0f, -28.0f);
    int listItemIndex = 0;
    for(auto& emailId : mReceivedEmails)
    {
        // Either reuse an existing label or create a new one.
        UILabel* label = nullptr;
        if(listItemIndex < mEmailListItems.size())
        {
            label = mEmailListItems[listItemIndex];
        }
        else
        {
            label = UIUtil::NewUIActorWithWidget<UILabel>(mEmailListWindow);
            label->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            label->GetOwner()->AddComponent<UIButton>();
            mEmailListItems.push_back(label);
        }
        label->GetRectTransform()->SetAnchoredPosition(listItemPos);
        
        // Font color depends on whether we've read this email already or not.
        bool alreadyReadThisEmail = mReadEmails.find(emailId) != mReadEmails.end();
        label->SetFont(alreadyReadThisEmail ? yellowFont : greenFont);
        
        // Set subject text.
        label->SetText(mAllEmails[emailId].subject);

        // On click, we show the full view of this email.
        label->GetOwner()->GetComponent<UIButton>()->SetPressCallback([emailId, listItemIndex, this](UIButton* button){
            ViewEmail(emailId, listItemIndex);
        });

        // Make sure the label is sized tightly.
        label->GetRectTransform()->SetSizeDelta(label->GetTextWidth(), label->GetFont()->GetGlyphHeight());

        // Move item pos to next line.
        listItemPos.y -= label->GetFont()->GetGlyphHeight();

        // Increment item index.
        ++listItemIndex;
    }
}

void SidneyEmail::ViewEmail(const std::string& emailId, int emailIndex)
{
    // Save index of email being viewed.
    mCurrentEmailIndex = emailIndex;

    // Update prev/next button availability based on index.
    mNextButton->GetButton()->SetCanInteract(mCurrentEmailIndex < mReceivedEmails.size() - 1);
    mPrevButton->GetButton()->SetCanInteract(mCurrentEmailIndex > 0);

    // Hide the list, show the single item view.
    mEmailWindow->SetActive(true);
    mEmailListWindow->SetActive(false);

    // Get the email.
    Email& email = mAllEmails[emailId];

    // Populate the metadata labels.
    mFromLabel->SetText(email.from);
    mToLabel->SetText(email.to);
    mCCLabel->SetText(email.cc);
    mDateLabel->SetText(email.date);
    mSubjectLabel->SetText(email.subject);

    // Reset scroll rect to top.
    mBodyScrollRect->SetNormalizedScrollValue(0.0f);

    // Populate body.
    {
        // First, hide any pre-existing labels.
        for(auto& bodyLabel : mBodyLabels)
        {
            bodyLabel->SetEnabled(false);
        }

        // Now activate each needed label, populating with correct text.
        int bodyLabelIndex = 0;
        Vector2 bodyItemPos(8.0f, -8.0f);
        Font* yellowFont = gAssetManager.LoadFont("SID_TEXT_14.FON");
        for(auto& bodyText : email.body)
        {
            // Either reuse an existing label or create a new one.
            UILabel* label = nullptr;
            if(bodyLabelIndex < mBodyLabels.size())
            {
                label = mBodyLabels[bodyLabelIndex];
            }
            else
            {
                label = UIUtil::NewUIActorWithWidget<UILabel>(mBodyScrollRect);
                label->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                label->GetRectTransform()->SetSizeDelta(510.0f, yellowFont->GetGlyphHeight());
                label->SetFont(yellowFont);
                label->SetVerticalAlignment(VerticalAlignment::Top);
                label->SetHorizontalOverflow(HorizontalOverflow::Wrap);
                mBodyLabels.push_back(label);
            }
            label->SetEnabled(true);
            label->GetRectTransform()->SetAnchoredPosition(bodyItemPos);

            // Set the text.
            label->SetText(bodyText);

            // Calculate text height.
            float textHeight = yellowFont->GetGlyphHeight();
            if(!bodyText.empty())
            {
                textHeight = label->GetTextHeight();
            }

            // Resize label height to fit the text snugly.
            label->GetRectTransform()->SetSizeDeltaY(textHeight);

            // Move next line down below this text.
            bodyItemPos.y -= textHeight;

            // Increment item index.
            ++bodyLabelIndex;
        }
    }

    // Mark this email as read.
    mReadEmails.insert(emailId);
}

void SidneyEmail::OnNextEmailButtonPressed()
{
    mCurrentEmailIndex = Math::Min(mCurrentEmailIndex + 1, mReceivedEmails.size() - 1);
    ViewEmail(mReceivedEmails[mCurrentEmailIndex], mCurrentEmailIndex);
}

void SidneyEmail::OnPrevEmailButtonPressed()
{
    mCurrentEmailIndex = Math::Max(mCurrentEmailIndex - 1, 0);
    ViewEmail(mReceivedEmails[mCurrentEmailIndex], mCurrentEmailIndex);
}

void SidneyEmail::OnReplyButtonPressed()
{
    // Response depends on the email.
    if(StringUtil::EqualsIgnoreCase(mReceivedEmails[mCurrentEmailIndex], "Email1"))
    {
        // "Noooo thank you."
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O1V0OPF1\", 1)");
    }
    else if(StringUtil::EqualsIgnoreCase(mReceivedEmails[mCurrentEmailIndex], "Email2"))
    {
        // "Don't want to get his hopes up"
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O240OPF1\", 1)");
    }
    else if(StringUtil::EqualsIgnoreCase(mReceivedEmails[mCurrentEmailIndex], "Email3"))
    {
        // "I want to write back, but I don't know what to say"
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O570OPF1\", 1)");
    }

    // There are some emails where no response occurs at all (such as the easter egg email).
}

void SidneyEmail::OnComposeButtonPressed()
{
    // As far as I know, Grace always says "I don't want to write anything right now."
    gActionManager.ExecuteSheepAction("wait StartDialogue(\"02OXI2ZZ51\", 1)");
}

void SidneyEmail::OnPrintButtonPressed()
{
    // As far as I know, Grace always says "I don't need to print it."
    gActionManager.ExecuteSheepAction("wait StartDialogue(\"02OXJ2ZZ51\", 1)");
}

void SidneyEmail::OnCloseButtonPressed()
{
    // Goes back to the email list.
    ShowEmailList();
}
