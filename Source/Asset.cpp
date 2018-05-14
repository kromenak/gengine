//
// Asset.cpp
//
// Clark Kromenaker
//
#include "Asset.h"

Asset::Asset(std::string name) : mName(name)
{
    
}

std::string Asset::GetNameNoExtension()
{
    // Find '.' closest to end of name and get substring without point and extension.
    for(int i = (int)mName.size() - 1; i >= 0; i--)
    {
        if(mName[i] == '.')
        {
            return mName.substr(0, i);
        }
    }
    return mName;
}
