//
//  Main.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#include "GEngine.h"

int main(int argc, const char * argv[])
{
    // Create and initialize the engine.
    GEngine engine;
    bool initSucceeded = engine.Initialize();
    
    // If initialization succeeded, we can "run" the engine.
    // Running will loop indefinitely until the game is quit by user/os.
    // If init fails, the program ends immediately.
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
