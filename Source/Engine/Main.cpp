//
// Clark Kromenaker
//
// Program point of entry for all platforms.
//
#include <cstdio>

#include <SDL.h>

#include "BuildEnv.h"
#include "GEngine.h"
#include "Log.h"
#include "TextEncode.h"

int main(int argc, char* argv[])
{
    // Take care of any runtime configuration to ensure the program supports UTF-8.
    Utf8::Enable();

    // Output version info.
    Log("--------------------");
    Logf("G-Engine v%s", PROJECT_VERSION);
    Log("--------------------");

    // Create the engine.
    GEngine engine;

    // If init succeeds, we can "run" the engine.
    // If init fails, the program ends immediately. Failing code will output an error of some kind.
    bool initSucceeded = engine.Initialize();
    if(initSucceeded)
    {
        engine.Run();
    }

    // Do the opposite of init and shut...it...down.
    engine.Shutdown();
    return 0;
}
