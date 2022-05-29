#include "SheepAPI_Rendering.h"

#include "Services.h"
#include "VideoPlayer.h"

using namespace std;

shpvoid PlayFullScreenMovie(const std::string& movieName)
{
    Services::Get<VideoPlayer>()->Play(movieName, true, true, AddWait());
    return 0;
}
RegFunc1(PlayFullScreenMovie, void, string, WAITABLE, REL_FUNC);

shpvoid PlayFullScreenMovieX(const std::string& movieName, int autoclose)
{
    Services::Get<VideoPlayer>()->Play(movieName, true, autoclose != 0 ? true : false, AddWait());
    return 0;
}
RegFunc2(PlayFullScreenMovieX, void, string, int, WAITABLE, REL_FUNC);

shpvoid PlayMovie(const std::string& movieName)
{
    Services::Get<VideoPlayer>()->Play(movieName, false, true, AddWait());
    return 0;
}
RegFunc1(PlayMovie, void, string, WAITABLE, REL_FUNC);
