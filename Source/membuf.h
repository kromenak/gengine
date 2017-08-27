//
//  membuf.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/26/17.
//

#pragma once
#include <streambuf>

using namespace std;

class membuf : public streambuf
{
public:
    
private:
    const char* const mBegin;
    const char* const mEnd;
    const char* mCurrent;
    
public:
    membuf(const char* data, unsigned int length);
    
private:
    int_type underflow();
    int_type uflow();
    int_type pbackfail(int_type ch);
    std::streamsize showmanyc();
};
