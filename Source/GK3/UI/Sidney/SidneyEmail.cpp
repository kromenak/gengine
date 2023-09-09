#include "SidneyEmail.h"

#include "AssetManager.h"
#include "AudioManager.h"
#include "SidneyUtil.h"
#include "Texture.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"

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
    SidneyUtil::CreateMenuBar(mRoot, "E-MAIL", 84.0f);

    // Create email list window.
    {

    }
    
    // Hide by default.
    Hide();
}

void SidneyEmail::Show()
{
    mRoot->SetActive(true);
}

void SidneyEmail::Hide()
{
    mRoot->SetActive(false);
}