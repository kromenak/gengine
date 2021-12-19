#include "imstream.h"

imstream::imstream(const char* data, unsigned int length) :
    std::istream(new membuf(data, length))
{

}

imstream::~imstream()
{
    delete rdbuf();
}
