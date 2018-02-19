//
// Stage.cpp
//
// Clark Kromenaker
//
#include "Stage.h"
#include <iostream>

Stage::Stage(std::string name, int day, int hour) :
    mGeneralName(name)
{
    //TODO: Maybe there should be a utility function to do this, or maybe enum is better or something?
    std::string ampm = (hour <= 11) ? "A" : "P";
    if(hour > 12)
    {
        hour -= 12;
    }
    
    mSpecificName = name + std::to_string(day) + std::to_string(hour) + ampm;
    std::cout << mGeneralName << ", " << mSpecificName << std::endl;
}
