#include "SheepAPI_Sound.h"

#include "AssetManager.h"
#include "AudioManager.h"
#include "ReportManager.h"
#include "SceneManager.h"
#include "SoundtrackPlayer.h"
#include "StringUtil.h"

using namespace std;

shpvoid EnableSound(const std::string& soundType)
{
    if(soundType.empty() || StringUtil::EqualsIgnoreCase(soundType, "global"))
    {
        gAudioManager.SetMuted(false);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "sfx"))
    {
        gAudioManager.SetMuted(AudioType::SFX, false);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "dialogue"))
    {
        gAudioManager.SetMuted(AudioType::VO, false);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "ambient"))
    {
        gAudioManager.SetMuted(AudioType::Ambient, false);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "music"))
    {
        gAudioManager.SetMuted(AudioType::Music, false);
    }
    else
    {
        gReportManager.Log("Error", StringUtil::Format("Error: unrecognized game sound type: %s.", soundType.c_str()));
        ExecError();
    }
    return 0;
}
RegFunc1(EnableSound, void, string, IMMEDIATE, DEV_FUNC);

shpvoid DisableSound(const std::string& soundType)
{
    if(soundType.empty() || StringUtil::EqualsIgnoreCase(soundType, "global"))
    {
        gAudioManager.SetMuted(true);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "sfx"))
    {
        gAudioManager.SetMuted(AudioType::SFX, true);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "dialogue"))
    {
        gAudioManager.SetMuted(AudioType::VO, true);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "ambient"))
    {
        gAudioManager.SetMuted(AudioType::Ambient, true);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "music"))
    {
        gAudioManager.SetMuted(AudioType::Music, true);
    }
    else
    {
        gReportManager.Log("Error", StringUtil::Format("Error: unrecognized game sound type: %s.", soundType.c_str()));
        ExecError();
    }
    return 0;
}
RegFunc1(DisableSound, void, string, IMMEDIATE, DEV_FUNC);

int GetVolume(const std::string& soundType)
{
    if(soundType.empty() || StringUtil::EqualsIgnoreCase(soundType, "global"))
    {
        return static_cast<int>(gAudioManager.GetMasterVolume() * 100);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "sfx"))
    {
        return static_cast<int>(gAudioManager.GetVolume(AudioType::SFX) * 100);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "dialogue"))
    {
        return static_cast<int>(gAudioManager.GetVolume(AudioType::VO) * 100);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "ambient"))
    {
        return static_cast<int>(gAudioManager.GetVolume(AudioType::Ambient) * 100);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "music"))
    {
        return static_cast<int>(gAudioManager.GetVolume(AudioType::Music) * 100);
    }
    else
    {
        gReportManager.Log("Error", StringUtil::Format("Error: unrecognized game sound type: %s.", soundType.c_str()));
        ExecError();
        return 0;
    }
}
RegFunc1(GetVolume, int, string, IMMEDIATE, DEV_FUNC);

shpvoid SetVolume(const std::string& soundType, int volume)
{
    // Clamp volume 0-100.
    volume = Math::Clamp(volume, 0, 100);

    // Internally, volumes are 0.0f - 1.0f. So, convert it.
    float volumeNormalized = (float)volume / 100.0f;

    // Aaaand set it.
    if(soundType.empty() || StringUtil::EqualsIgnoreCase(soundType, "global"))
    {
        gAudioManager.SetMasterVolume(volumeNormalized);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "sfx"))
    {
        gAudioManager.SetVolume(AudioType::SFX, volumeNormalized);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "dialogue"))
    {
        gAudioManager.SetVolume(AudioType::VO, volumeNormalized);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "ambient"))
    {
        gAudioManager.SetVolume(AudioType::Ambient, volumeNormalized);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "music"))
    {
        gAudioManager.SetVolume(AudioType::Music, volumeNormalized);
    }
    else
    {
        gReportManager.Log("Error", StringUtil::Format("Error: unrecognized game sound type: %s.", soundType.c_str()));
        ExecError();
    }
    return 0;
}
RegFunc2(SetVolume, void, string, int, IMMEDIATE, DEV_FUNC);

shpvoid PlaySound(const std::string& soundName)
{
    Audio* audio = gAssetManager.LoadAudio(soundName, AssetScope::Scene);
    if(audio != nullptr)
    {
        gAudioManager.PlaySFX(audio, AddWait());
    }
    return 0;
}
RegFunc1(PlaySound, void, string, WAITABLE, REL_FUNC);

shpvoid StopSound(const std::string& soundName)
{
    // For a sound to play, it must be loaded already anyway.
    // And if it's null, the Stop function handles that.
    gAudioManager.Stop(gAssetManager.LoadAudio(soundName, AssetScope::Scene));
    return 0;
}
RegFunc1(StopSound, void, string, IMMEDIATE, REL_FUNC);

shpvoid StopAllSounds()
{
    gAudioManager.StopAll();
    return 0;
}
RegFunc0(StopAllSounds, void, IMMEDIATE, REL_FUNC);

shpvoid PlaySoundTrack(const std::string& soundtrackName)
{
    Soundtrack* soundtrack = gAssetManager.LoadSoundtrack(soundtrackName, AssetScope::Scene);
    if(soundtrack != nullptr)
    {
        gSceneManager.GetScene()->GetSoundtrackPlayer()->Play(soundtrack);
    }
    return 0;
}
RegFunc1(PlaySoundTrack, void, string, WAITABLE, REL_FUNC);

shpvoid StopSoundTrack(const std::string& soundtrackName)
{
    // Soundtrack must already be loaded to be playing. So, just load and pass in pointer.
    // If it is null, Stop handles that for us.
    gSceneManager.GetScene()->GetSoundtrackPlayer()->Stop(gAssetManager.LoadSoundtrack(soundtrackName, AssetScope::Scene));
    return 0;
}
RegFunc1(StopSoundTrack, void, string, IMMEDIATE, REL_FUNC);

shpvoid StopAllSoundTracks()
{
    gSceneManager.GetScene()->GetSoundtrackPlayer()->StopAll();
    return 0;
}
RegFunc0(StopAllSoundTracks, void, IMMEDIATE, REL_FUNC);
