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

streampos membuf::seekoff(streamoff off, ios_base::seekdir way, ios_base::openmode which)
{
    if(way == ios_base::seekdir::beg)
    {
        mCurrent = mBegin + off;
        if(mCurrent > mEnd)
        {
            mCurrent = mEnd;
        }
    }
    else if(way == ios_base::seekdir::cur)
    {
        mCurrent += off;
        if(mCurrent > mEnd)
        {
            mCurrent = mEnd;
        }
    }
    else if(way == ios_base::seekdir::end)
    {
        mCurrent = mEnd - off;
        if(mCurrent < mBegin)
        {
            mCurrent = mBegin;
        }
    }
    return mCurrent - mBegin;
}

streampos membuf::seekpos(streampos pos, ios_base::openmode which)
{
    mCurrent = mBegin + pos;
    if(mCurrent > mEnd)
    {
        mCurrent = mEnd;
    }
    return mCurrent - mBegin;
}

streamsize membuf::showmanyc()
{
    return mEnd - mCurrent;
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


