//
//  Main.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#include "GEngine.h"

int main(int argc, const char * argv[])
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
