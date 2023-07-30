#include "SheepAPI_Dialogue.h"

#include "Animator.h"
#include "AssetManager.h"
#include "DialogueManager.h"
#include "GK3UI.h"
#include "SceneManager.h"

using namespace std;

shpvoid StartVoiceOver(const std::string& dialogueName, int numLines)
{
    std::string yakName = "E" + dialogueName;
    Animation* yak = gAssetManager.LoadYak(yakName, AssetScope::Scene);
    
    AnimParams params;
    params.animation = yak;
    params.finishCallback = AddWait();
    params.isYak = true;
    gSceneManager.GetScene()->GetAnimator()->Start(params);
    return 0;
}
RegFunc2(StartVoiceOver, void, string, int, WAITABLE, REL_FUNC);

shpvoid StartYak(const std::string& yakAnimationName)
{
    Animation* yak = gAssetManager.LoadYak(yakAnimationName, AssetScope::Scene);

    AnimParams params;
    params.animation = yak;
    params.finishCallback = AddWait();
    params.isYak = true;
    gSceneManager.GetScene()->GetAnimator()->Start(params);
    return 0;
}
RegFunc1(StartYak, void, string, WAITABLE, DEV_FUNC);

shpvoid StartDialogue(const std::string& licensePlate, int numLines)
{
    gDialogueManager.StartDialogue(licensePlate, numLines, true, AddWait());
    return 0;
}
RegFunc2(StartDialogue, void, string, int, WAITABLE, REL_FUNC);

shpvoid StartDialogueNoFidgets(const std::string& licensePlate, int numLines)
{
    gDialogueManager.StartDialogue(licensePlate, numLines, false, AddWait());
    return 0;
}
RegFunc2(StartDialogueNoFidgets, void, string, int, WAITABLE, REL_FUNC);

shpvoid ContinueDialogue(int numLines)
{
    gDialogueManager.ContinueDialogue(numLines, true, AddWait());
    return 0;
}
RegFunc1(ContinueDialogue, void, int, WAITABLE, REL_FUNC);

shpvoid ContinueDialogueNoFidgets(int numLines)
{
    gDialogueManager.ContinueDialogue(numLines, false, AddWait());
    return 0;
}
RegFunc1(ContinueDialogueNoFidgets, void, int, WAITABLE, REL_FUNC);

shpvoid SetDefaultDialogueCamera(const std::string& cameraName)
{
    return 0;
}
RegFunc1(SetDefaultDialogueCamera, void, string, IMMEDIATE, REL_FUNC);

shpvoid SetConversation(const std::string& conversationName)
{
    gDialogueManager.SetConversation(conversationName, AddWait());
    return 0;
}
RegFunc1(SetConversation, void, string, WAITABLE, REL_FUNC);

shpvoid EndConversation()
{
    gDialogueManager.EndConversation(AddWait());
    return 0;
}
RegFunc0(EndConversation, void, WAITABLE, REL_FUNC);

shpvoid AddCaptionDefault(const std::string& captionText)
{
    gGK3UI.AddCaption(captionText);
    return 0;
}
RegFunc1(AddCaptionDefault, void, string, IMMEDIATE, REL_FUNC);

shpvoid AddCaptionEgo(const std::string& captionText)
{
    gGK3UI.AddCaption(captionText, Scene::GetEgoName());
    return 0;
}
RegFunc1(AddCaptionEgo, void, string, IMMEDIATE, REL_FUNC);

shpvoid AddCaptionVoiceOver(const std::string& captionText)
{
    gGK3UI.AddCaption(captionText, "VOICEOVER");
    return 0;
}
RegFunc1(AddCaptionVoiceOver, void, string, IMMEDIATE, REL_FUNC);

shpvoid ClearCaptionText()
{
    gGK3UI.HideAllCaptions();
    return 0;
}
RegFunc0(ClearCaptionText, void, IMMEDIATE, DEV_FUNC);