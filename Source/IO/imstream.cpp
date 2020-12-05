//
//  imstream.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 8/26/17.
//
#include "imstream.h"

imstream::imstream(const char* data, unsigned int length) :
    std::istream(new membuf(data, length))
{

}

imstream::~imstream()
{
    delete rdbuf();
}
