//
// Clark Kromenaker
//
// Sheep SysFuncs related audio and sound effects.
//
#pragma once
#include "SheepSysFunc.h"

shpvoid EnableSound(const std::string& soundType); // DEV
shpvoid DisableSound(const std::string& soundType); // DEV

int GetVolume(const std::string& soundType); // DEV
shpvoid SetVolume(const std::string& soundType, int volume); // DEV

shpvoid PlaySound(const std::string& soundName); // WAIT
shpvoid StopSound(const std::string& soundName);
shpvoid StopAllSounds();

shpvoid PlaySoundTrack(const std::string& soundtrackName); // WAIT
shpvoid StopSoundTrack(const std::string& soundtrackName); // WAIT
shpvoid StopAllSoundTracks();