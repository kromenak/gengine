//
// Clark Kromenaker
//
// Sheep SysFuncs related to controlling Animations.
//
#pragma once
#include "SheepSysFunc.h"

// ANIMATION SETTINGS
shpvoid EnableInterpolation(); // DEV
shpvoid DisableInterpolation(); // DEV

shpvoid DumpAnimator(); // DEV

// ANIMATIONS
shpvoid StartAnimation(const std::string& animationName); // WAIT
shpvoid LoopAnimation(const std::string& animationName);
shpvoid StopAnimation(const std::string& animationName);
shpvoid StopAllAnimations(); // DEV

shpvoid StartMoveAnimation(const std::string& animName); // WAIT
shpvoid StartMom(const std::string& momAnimationName); // WAIT

shpvoid AnimEvent(const std::string& eventType, const std::string& eventData); // DEV (NOTE: Appears to never be used in final game?)

// MORPH ANIMATIONS
shpvoid StartMorphAnimation(const std::string& animationName, int animStartFrame, int morphFrames); // WAIT
shpvoid StopMorphAnimation(const std::string& animationName);
shpvoid StopAllMorphAnimations(); // DEV
