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
    // Buffer management and positioning
    streampos seekoff(streamoff off, ios_base::seekdir way,
                      ios_base::openmode which = ios_base::in | ios_base::out);
    streampos seekpos(streampos pos,
                      ios_base::openmode which = ios_base::in | ios_base::out);
    
    // Input functions (get)
    streamsize showmanyc();
    int_type underflow();
    int_type uflow();
    int_type pbackfail(int_type ch);
};
