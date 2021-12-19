//
// Clark Kromenaker
//
// A stream buffer variation that uses an arbitrary byte array in memory.
// C++ provides stream buffers for file and string data, but not a byte array.
//
#pragma once
#include <streambuf>

using namespace std;

class membuf : public streambuf
{
public:
    membuf(const char* data, unsigned int length);
    
private:
	const char* const mBegin;
	const char* const mEnd;
	const char* mCurrent;
	
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
