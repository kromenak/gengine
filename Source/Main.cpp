//
// Clark Kromenaker
//
// Program point of entry for all platforms.
//
#include <cstdio>

#define SDL_MAIN_HANDLED // For Windows: we provide our own main, so use that!
#include "GEngine.h"

int main(int argc, const char* argv[])
{
    printf("Application Start!\n");

    // Create the engine.
	GEngine engine;
	
    // If init succeeds, we can "run" the engine.
    // If init fails, the program ends immediately.
	bool initSucceeded = engine.Initialize();
    if(initSucceeded)
    {
        engine.Run();
    }
    else
    {
        //TODO: Output reason engine failed to initialize.
    }
    
    // Do the opposite of init and shut...it...down.
    engine.Shutdown();
    return 0;
}
