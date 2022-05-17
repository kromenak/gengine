#include "SheepAPI_Sound.h"

#include "AudioManager.h"
#include "GEngine.h"
#include "Scene.h"
#include "Services.h"
#include "SoundtrackPlayer.h"
#include "StringUtil.h"

using namespace std;

shpvoid EnableSound(const std::string& soundType)
{
    if(soundType.empty() || StringUtil::EqualsIgnoreCase(soundType, "global"))
    {
        Services::GetAudio()->SetMuted(false);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "sfx"))
    {
        Services::GetAudio()->SetMuted(AudioType::SFX, false);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "dialogue"))
    {
        Services::GetAudio()->SetMuted(AudioType::VO, false);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "ambient"))
    {
        Services::GetAudio()->SetMuted(AudioType::Ambient, false);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "music"))
    {
        Services::GetAudio()->SetMuted(AudioType::Music, false);
    }
    else
    {
        Services::GetReports()->Log("Error", StringUtil::Format("Error: unrecognized game sound type: %s.", soundType.c_str()));
        ExecError();
    }
    return 0;
}
RegFunc1(EnableSound, void, string, IMMEDIATE, DEV_FUNC);

shpvoid DisableSound(const std::string& soundType)
{
    if(soundType.empty() || StringUtil::EqualsIgnoreCase(soundType, "global"))
    {
        Services::GetAudio()->SetMuted(true);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "sfx"))
    {
        Services::GetAudio()->SetMuted(AudioType::SFX, true);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "dialogue"))
    {
        Services::GetAudio()->SetMuted(AudioType::VO, true);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "ambient"))
    {
        Services::GetAudio()->SetMuted(AudioType::Ambient, true);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "music"))
    {
        Services::GetAudio()->SetMuted(AudioType::Music, true);
    }
    else
    {
        Services::GetReports()->Log("Error", StringUtil::Format("Error: unrecognized game sound type: %s.", soundType.c_str()));
        ExecError();
    }
    return 0;
}
RegFunc1(DisableSound, void, string, IMMEDIATE, DEV_FUNC);

int GetVolume(const std::string& soundType)
{
    if(soundType.empty() || StringUtil::EqualsIgnoreCase(soundType, "global"))
    {
        return static_cast<int>(Services::GetAudio()->GetMasterVolume() * 100);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "sfx"))
    {
        return static_cast<int>(Services::GetAudio()->GetVolume(AudioType::SFX) * 100);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "dialogue"))
    {
        return static_cast<int>(Services::GetAudio()->GetVolume(AudioType::VO) * 100);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "ambient"))
    {
        return static_cast<int>(Services::GetAudio()->GetVolume(AudioType::Ambient) * 100);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "music"))
    {
        return static_cast<int>(Services::GetAudio()->GetVolume(AudioType::Music) * 100);
    }
    else
    {
        Services::GetReports()->Log("Error", StringUtil::Format("Error: unrecognized game sound type: %s.", soundType.c_str()));
        ExecError();
        return 0;
    }
}
RegFunc1(GetVolume, void, string, IMMEDIATE, DEV_FUNC);

shpvoid SetVolume(const std::string& soundType, int volume)
{
    // Clamp volume 0-100.
    volume = Math::Clamp(volume, 0, 100);

    // Internally, volumes are 0.0f - 1.0f. So, convert it.
    float volumeNormalized = (float)volume / 100.0f;

    // Aaaand set it.
    if(soundType.empty() || StringUtil::EqualsIgnoreCase(soundType, "global"))
    {
        Services::GetAudio()->SetMasterVolume(volumeNormalized);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "sfx"))
    {
        Services::GetAudio()->SetVolume(AudioType::SFX, volumeNormalized);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "dialogue"))
    {
        Services::GetAudio()->SetVolume(AudioType::VO, volumeNormalized);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "ambient"))
    {
        Services::GetAudio()->SetVolume(AudioType::Ambient, volumeNormalized);
    }
    else if(StringUtil::EqualsIgnoreCase(soundType, "music"))
    {
        Services::GetAudio()->SetVolume(AudioType::Music, volumeNormalized);
    }
    else
    {
        Services::GetReports()->Log("Error", StringUtil::Format("Error: unrecognized game sound type: %s.", soundType.c_str()));
        ExecError();
    }
    return 0;
}
RegFunc2(SetVolume, void, string, int, IMMEDIATE, DEV_FUNC);

shpvoid PlaySound(const std::string& soundName)
{
    Audio* audio = Services::GetAssets()->LoadAudio(soundName);
    if(audio != nullptr)
    {
        SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
        Services::GetAudio()->PlaySFX(audio, currentThread->AddWait());
    }
    return 0;
}
RegFunc1(PlaySound, void, string, WAITABLE, REL_FUNC);

shpvoid StopSound(const std::string& soundName)
{
    // For a sound to play, it must be loaded already anyway.
    // And if it's null, the Stop function handles that.
    Services::GetAudio()->Stop(Services::GetAssets()->LoadAudio(soundName));
    return 0;
}
RegFunc1(StopSound, void, string, IMMEDIATE, REL_FUNC);

shpvoid StopAllSounds()
{
    Services::GetAudio()->StopAll();
    return 0;
}
RegFunc0(StopAllSounds, void, IMMEDIATE, REL_FUNC);

shpvoid PlaySoundTrack(const std::string& soundtrackName)
{
    Soundtrack* soundtrack = Services::GetAssets()->LoadSoundtrack(soundtrackName);
    if(soundtrack != nullptr)
    {
        GEngine::Instance()->GetScene()->GetSoundtrackPlayer()->Play(soundtrack);
    }
    return 0;
}
RegFunc1(PlaySoundTrack, void, string, WAITABLE, REL_FUNC);

shpvoid StopSoundTrack(const std::string& soundtrackName)
{
    // Soundtrack must already be loaded to be playing. So, just load and pass in pointer.
    // If it is null, Stop handles that for us.
    GEngine::Instance()->GetScene()->GetSoundtrackPlayer()->Stop(Services::GetAssets()->LoadSoundtrack(soundtrackName));
    return 0;
}
RegFunc1(StopSoundTrack, void, string, IMMEDIATE, REL_FUNC);

shpvoid StopAllSoundTracks()
{
    GEngine::Instance()->GetScene()->GetSoundtrackPlayer()->StopAll();
    return 0;
}
RegFunc0(StopAllSoundTracks, void, IMMEDIATE, REL_FUNC);