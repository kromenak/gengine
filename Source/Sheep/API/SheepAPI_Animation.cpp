#include "SheepAPI_Animation.h"

#include "Animator.h"
#include "AssetManager.h"
#include "Localizer.h"
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
        Scene::GetGlobalAnimator()->Stop(animation);
    }
    return 0;
}
RegFunc1(StopAnimation, void, string, IMMEDIATE, REL_FUNC);

shpvoid StopAllAnimations()
{
    gSceneManager.GetScene()->GetAnimator()->StopAll();
    Scene::GetGlobalAnimator()->StopAll();
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
    // MOM assets are localized, but the passed in name should not have a language prefix. We add the "E" here.

    // MOM anims are (unfortunately) localized, even though they don't really have to be.
    // The only aspect of a MOM that warrants localization is YAK file names embedded in the asset, and we can programmatically fix that for new languages.
    // However, official localization (such as French) do localize these assets, so we must take that into account.

    // First, try to load the localized version of the asset.
    Animation* animation = gAssetManager.LoadMomAnimation(Localizer::GetLanguagePrefix() + momAnimationName, AssetScope::Scene);
    if(animation == nullptr)
    {
        // If we can't load that, fall back on the English version. For unofficial localizations, there is no need to localize these files.
        animation = gAssetManager.LoadMomAnimation("E" + momAnimationName, AssetScope::Scene);
    }
    if(animation != nullptr)
    {
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
