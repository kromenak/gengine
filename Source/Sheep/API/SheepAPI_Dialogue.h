//
// Clark Kromenaker
//
// Sheep SysFuncs related to dialogue and voice-overs.
//
#pragma once
#include "SheepSysFunc.h"

// VO & TALKING
shpvoid StartVoiceOver(const std::string& dialogueName, int numLines); // WAIT
shpvoid StartYak(const std::string& yakAnimName); // WAIT, DEV

// DIALOGUE/CONVERSATIONS
shpvoid StartDialogue(const std::string& dialogueName, int numLines); // WAIT
shpvoid StartDialogueNoFidgets(const std::string& dialogueName, int numLines); // WAIT
shpvoid ContinueDialogue(int numLines); // WAIT
shpvoid ContinueDialogueNoFidgets(int numLines); // WAIT

shpvoid SetDefaultDialogueCamera(const std::string& cameraName);

shpvoid SetConversation(const std::string& conversationName); // WAIT
shpvoid EndConversation(); // WAIT

// CAPTIONS
shpvoid AddCaptionDefault(const std::string& captionText);
shpvoid AddCaptionEgo(const std::string& captionText);
shpvoid AddCaptionVoiceOver(const std::string& captionText);
shpvoid ClearCaptionText(); // DEV
