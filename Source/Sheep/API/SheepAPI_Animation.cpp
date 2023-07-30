#include "SheepAPI_Animation.h"

#include "Animator.h"
#include "AssetManager.h"
#include "ReportManager.h"
#include "SceneManager.h"

using namespace std;

shpvoid StartAnimation(const std::string& animationName)
{
    Animation* animation = gAssetManager.LoadAnimation(animationName, AssetScope::Scene);
    if(animation == nullptr)
    {
        gReportManager.Log("Error", "gk3 animation '" + animationName + ".anm' not found.");
        return 0;
    }
    
    gSceneManager.GetScene()->GetAnimator()->Start(animation, AddWait());
    return 0;
}
RegFunc1(StartAnimation, void, string, WAITABLE, REL_FUNC);

shpvoid LoopAnimation(const std::string& animationName)
{
    Animation* animation = gAssetManager.LoadAnimation(animationName, AssetScope::Scene);
    if(animation != nullptr)
    {
        AnimParams params;
        params.animation = animation;
        params.loop = true;
        gSceneManager.GetScene()->GetAnimator()->Start(params);
    }
    return 0;
}
RegFunc1(LoopAnimation, void, string, IMMEDIATE, REL_FUNC);

shpvoid StopAnimation(const std::string& animationName)
{
    Animation* animation = gAssetManager.LoadAnimation(animationName, AssetScope::Scene);
    if(animation != nullptr)
    {
        gSceneManager.GetScene()->GetAnimator()->Stop(animation);
    }
    return 0;
}
RegFunc1(StopAnimation, void, string, IMMEDIATE, REL_FUNC);

shpvoid StopAllAnimations()
{
    gSceneManager.GetScene()->GetAnimator()->StopAll();
    return 0;
}
RegFunc0(StopAllAnimations, void, IMMEDIATE, DEV_FUNC);

shpvoid StartMoveAnimation(const std::string& animationName)
{
    Animation* animation = gAssetManager.LoadAnimation(animationName, AssetScope::Scene);
    if(animation != nullptr)
    {
        AnimParams animParams;
        animParams.animation = animation;
        animParams.allowMove = true;
        animParams.finishCallback = AddWait();
        gSceneManager.GetScene()->GetAnimator()->Start(animParams);
    }
    return 0;
}
RegFunc1(StartMoveAnimation, void, string, WAITABLE, REL_FUNC);

shpvoid StartMom(const std::string& momAnimationName)
{
    // Mom animation assets have a language prefix (e.g. "E" for English).
    // So, let's add that here.
    Animation* animation = gAssetManager.LoadMomAnimation("E" + momAnimationName, AssetScope::Scene);
    if(animation != nullptr)
    {
        //TODO: Any need to send flag that this is a MOM animation file? The formats/uses seem identical.
        gSceneManager.GetScene()->GetAnimator()->Start(animation, AddWait());
    }
    return 0;
}
RegFunc1(StartMom, void, string, WAITABLE, REL_FUNC);

/*
shpvoid StartMorphAnimation(std::string animationName, int animStartFrame, int morphFrames)
{
    std::cout << "StartMorphAnimation" << std::endl;
    return 0;
}
RegFunc3(StartMorphAnimation, void, string, int, int, WAITABLE, REL_FUNC);

shpvoid StopMorphAnimation(std::string animationName)
{
    std::cout << "StopMorphAnimation" << std::endl;
    return 0;
}
RegFunc1(StopMorphAnimation, void, string, IMMEDIATE, REL_FUNC);

//StopAllMorphAnimations
*/
