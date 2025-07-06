#include "FlagSet.h"

#include "ReportManager.h"

bool FlagSet::Get(const std::string& flag) const
{
    // If the flag exists, it implies a "true" value.
    // Absence of flag implies "false" value.
    auto it = mFlags.find(flag);
    return it != mFlags.end();
}

void FlagSet::Set(const std::string& flag)
{
    // Doesn't matter whether we are setting an already set flag.
    mFlags.insert(flag);
}

void FlagSet::Clear(const std::string& flag)
{
    // Erase the flag from the container to "clear" it.
    auto it = mFlags.find(flag);
    if(it != mFlags.end())
    {
        mFlags.erase(it);
    }
}

void FlagSet::Toggle(const std::string& flag)
{
    if(Get(flag))
    {
        Clear(flag);
    }
    else
    {
        Set(flag);
    }
}

void FlagSet::Dump(const std::string& label) const
{
    // Add header.
    std::string dump;
    if(!label.empty())
    {
        dump += "Dumping " + label + " flags...\n";
    }
    else
    {
        dump += "Dumping flags...\n";
    }

    // Extra space if we actually have flags.
    if(!mFlags.empty())
    {
        dump += "\n";
    }

    // Add each flag to the dump.
    //TODO: Our output differs from the original game in two ways.
    //TODO: 1) We don't prepopulate (aka hard-code) every possible flag up-front.
    //TODO: 2) Flags aren't output if their values are false (since they won't be present in the set).
    for(auto& entry : mFlags)
    {
        dump += StringUtil::Format("flag \"%s\" is true\n", entry.c_str());
    }
    gReportManager.Log("Dump", dump);
}