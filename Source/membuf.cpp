//
//  membuf.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 8/26/17.
//

#include "membuf.h"

membuf::membuf(const char* data, unsigned int length) :
    mBegin(data),
    mEnd(data + length),
    mCurrent(data)
{
    
}

membuf::int_type membuf::underflow()
{
    if(mCurrent == mEnd)
    {
        return traits_type::eof();
    }
    return traits_type::to_int_type(*mCurrent);
}

membuf::int_type membuf::uflow()
{
    if(mCurrent == mEnd)
    {
        return traits_type::eof();
    }
    return traits_type::to_int_type(*mCurrent++);
}

membuf::int_type membuf::pbackfail(int_type ch)
{
    if(mCurrent == mBegin || (ch != traits_type::eof() && ch != mCurrent[-1]))
    {
        return traits_type::eof();
    }
    return traits_type::to_int_type(*--mCurrent);
}

std::streamsize membuf::showmanyc()
{
    return mEnd - mCurrent;
}
