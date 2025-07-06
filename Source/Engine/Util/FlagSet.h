//
// Clark Kromenaker
//
// A set of flags.
//
#pragma once
#include "StringUtil.h"

class FlagSet
{
public:
    bool Get(const std::string& flag) const;
    void Set(const std::string& flag);
    void Clear(const std::string& flag);
    void Toggle(const std::string& flag);

    void Dump(const std::string& label = "") const;

    std::string_set_ci& GetFlags() { return mFlags; }

private:
    // The flags.
    std::string_set_ci mFlags;
};