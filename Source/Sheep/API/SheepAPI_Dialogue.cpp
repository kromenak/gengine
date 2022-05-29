#include "SheepAPI_Dialogue.h"

#include "Animator.h"
#include "DialogueManager.h"
#include "GEngine.h"
#include "Scene.h"
#include "Services.h"

using namespace std;

shpvoid StartVoiceOver(const std::string& dialogueName, int numLines)
{
    std::string yakName = "E" + dialogueName;
    Animation* yak = Services::GetAssets()->LoadYak(yakName);
    
    AnimParams params;
    params.animation = yak;
    params.finishCallback = AddWait();
    params.isYak = true;
    GEngine::Instance()->GetScene()->GetAnimator()->Start(params);
    return 0;
}
RegFunc2(StartVoiceOver, void, string, int, WAITABLE, REL_FUNC);

shpvoid StartYak(const std::string& yakAnimationName)
{
    Animation* yak = Services::GetAssets()->LoadYak(yakAnimationName);

    AnimParams params;
    params.animation = yak;
    params.finishCallback = AddWait();
    params.isYak = true;
    GEngine::Instance()->GetScene()->GetAnimator()->Start(params);
    return 0;
}
RegFunc1(StartYak, void, string, WAITABLE, DEV_FUNC);

shpvoid StartDialogue(const std::string& licensePlate, int numLines)
{
    Services::Get<DialogueManager>()->StartDialogue(licensePlate, numLines, true, AddWait());
    return 0;
}
RegFunc2(StartDialogue, void, string, int, WAITABLE, REL_FUNC);

shpvoid StartDialogueNoFidgets(const std::string& licensePlate, int numLines)
{
    Services::Get<DialogueManager>()->StartDialogue(licensePlate, numLines, false, AddWait());
    return 0;
}
RegFunc2(StartDialogueNoFidgets, void, string, int, WAITABLE, REL_FUNC);

shpvoid ContinueDialogue(int numLines)
{
    Services::Get<DialogueManager>()->ContinueDialogue(numLines, true, AddWait());
    return 0;
}
RegFunc1(ContinueDialogue, void, int, WAITABLE, REL_FUNC);

shpvoid ContinueDialogueNoFidgets(int numLines)
{
    Services::Get<DialogueManager>()->ContinueDialogue(numLines, false, AddWait());
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
    Services::Get<DialogueManager>()->SetConversation(conversationName, AddWait());
    return 0;
}
RegFunc1(SetConversation, void, string, WAITABLE, REL_FUNC);

shpvoid EndConversation()
{
    Services::Get<DialogueManager>()->EndConversation(AddWait());
    return 0;
}
RegFunc0(EndConversation, void, WAITABLE, REL_FUNC);
