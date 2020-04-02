//
//  Main.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#define SDL_MAIN_HANDLED // For Windows: we provide our own main, so use that!
#include "GEngine.h"

int main(int argc, const char* argv[])
{
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
