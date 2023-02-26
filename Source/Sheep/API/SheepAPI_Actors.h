//
// Clark Kromenaker
//
// Sheep SysFuncs related to controlling Actors (and Models/Props).
//
#pragma once
#include "SheepSysFunc.h"

// EYES
shpvoid Blink(const std::string& actorName);
shpvoid BlinkX(const std::string& actorName, const std::string& blinkAnim);

shpvoid EnableEyeJitter(const std::string& actorName);
shpvoid DisableEyeJitter(const std::string& actorName);
shpvoid EyeJitter(const std::string& actorName);

shpvoid Glance(const std::string& actorName, int percentX, int percentY, int durationMs); // WAIT
shpvoid GlanceX(const std::string& actorName, int leftPercentX, int leftPercentY,
                int rightPercentX, int rightPercentY, int durationMs); // WAIT

shpvoid SetEyeOffsets(const std::string& actorName, float leftX, float leftY,
                      float rightX, float rightY); // DEV

// MOOD & EXPRESSION
shpvoid SetMood(const std::string& actorName, const std::string& moodName);
shpvoid ClearMood(const std::string& actorName);

shpvoid Expression(const std::string& actorName, const std::string& expression);

// FIDGETS
shpvoid SetIdleGAS(const std::string& actorName, const std::string& gasName); // WAIT
shpvoid SetListenGAS(const std::string& actorName, const std::string& gasName); // WAIT
shpvoid SetTalkGAS(const std::string& actorName, const std::string& gasName); // WAIT

shpvoid StartIdleFidget(const std::string& actorName); // WAIT
shpvoid StartListenFidget(const std::string& actorName); // WAIT
shpvoid StartTalkFidget(const std::string& actorName); // WAIT
shpvoid StopFidget(const std::string& actorName); // WAIT

// WALKING
shpvoid ActionWaitClearRegion(const std::string& actorName, int regionId, float destAccuracy, const std::string& exitPosition);

shpvoid SetWalkAnim(const std::string& actorName, const std::string& start, const std::string& cont,
                    const std::string& startTurnLeft, const std::string& startTurnRight);


// TURNS & LOOK-ATS
shpvoid TurnHead(const std::string& actorName, int percentX, int percentY, int durationMs); // WAIT
shpvoid TurnToModel(const std::string& actorName, const std::string& modelName); // WAIT

shpvoid LookitActor(const std::string& actorName, const std::string& otherActorName,
                    const std::string& componentsSpec, float durationSec);
shpvoid LookitActorQuick(const std::string& actorName, const std::string& otherActorName,
                         const std::string& componentsSpec, float durationSec);

shpvoid LookitModel(const std::string& actorName, const std::string& modelName,
                    const std::string& componentsSpec, float durationSec);
shpvoid LookitModelQuick(const std::string& actorName, const std::string& modelName,
                         const std::string& componentsSpec, float durationSec);
shpvoid LookitModelX(const std::string& actorName, const std::string& modelName, int mesh,
                     const std::string& boxModifier, float offsetX, float offsetY, float offsetZ,
                     const std::string& componentsSpec, float durationSec);
shpvoid LookitModelQuickX(const std::string& actorName, const std::string& modelName, int mesh,
                          const std::string& boxModifier, float offsetX, float offsetY, float offsetZ,
                          const std::string& componentsSpec, float durationSec);

shpvoid LookitNoun(const std::string& actorName, const std::string& nounName,
                   const std::string& componentsSpec, float durationSec);
shpvoid LookitNounQuick(const std::string& actorName, const std::string& nounName,
                        const std::string& componentsSpec, float durationSec);

shpvoid LookitPoint(const std::string& actorName, float x, float y, float z,
                    const std::string& componentsSpec, float durationSec);

shpvoid LookitCameraAngle(const std::string& actorName, const std::string& cameraAngleName,
                          const std::string& componentsSpec, float durationSec); // DEV
shpvoid LookitMouse(const std::string& actorName, const std::string& componentsSpec, float durationSec); // DEV
shpvoid LookitPlayer(const std::string& actorName, const std::string& componentsSpec, float durationSec); // DEV

shpvoid LookitCancel(const std::string& actorName);

shpvoid LookitLock(const std::string& actorName, const std::string& componentsSpec, float durationSec);
shpvoid LookitUnlock(const std::string& actorName, const std::string& componentsSpec);

// MODELS
shpvoid SetModelShadowTexture(std::string modelName, std::string textureName);
shpvoid ClearModelShadowTexture(std::string modelName);
shpvoid DumpModel(std::string modelName); // DEV

// PROPS
shpvoid SetPropGas(std::string modelName, std::string gasName);
shpvoid ClearPropGas(std::string modelName);

shpvoid StartPropFidget(std::string modelName);
shpvoid StopPropFidget(std::string modelName);
