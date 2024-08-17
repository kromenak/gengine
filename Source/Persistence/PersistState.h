//
// Clark Kromenaker
//
// A class used for saving/loading data to/from a save file on disk.
// The "Xfer" functions are used to transfer the data.
//
#pragma once
#include <cstdint>
#include <set>

#include <string>
#include <unordered_map>

#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "StringUtil.h" // for string maps

class IniParser;
class IniWriter;

#define PERSIST_VAR(var) #var, var

enum class PersistMode : uint8_t
{
    Save,
    Load
};

enum class PersistFormat : uint8_t
{
    Text,
    Binary
};

class PersistState
{
public:
    PersistState(const char* filePath, PersistFormat format, PersistMode mode);
    ~PersistState();

    // Strings
    void Xfer(const char* name, char* value, size_t valueSize);
    void Xfer(const char* name, std::string& value);

    // Integers
    void Xfer(const char* name, bool& value);

    //TODO: int8_t/uint8_t

    void Xfer(const char* name, int16_t& value);
    void Xfer(const char* name, uint16_t& value);

    void Xfer(const char* name, int32_t& value);
    void Xfer(const char* name, uint32_t& value);

    //TODO: int64_t/uint64_t

    // Floats
    void Xfer(const char* name, float& value);
    //TODO: Double?

    // Collections
    template<typename T> void Xfer(const char* name, std::set<T>& set);
    template<typename T> void Xfer(const char* name, std::unordered_map<std::string, T>& map);
    template<typename T> void Xfer(const char* name, std::string_map_ci<T>& map);
    void Xfer(const char* name, std::string_set_ci& set);

private:
    PersistMode mMode = PersistMode::Save;
    PersistFormat mFormat = PersistFormat::Text;

    BinaryReader* mBinaryReader = nullptr;
    IniParser* mIniReader = nullptr;

    BinaryWriter* mBinaryWriter = nullptr;
    IniWriter* mIniWriter = nullptr;
};

template<typename T>
inline void PersistState::Xfer(const char* name, std::set<T>& set)
{
    if(mBinaryReader != nullptr)
    {
        set.clear();
        uint64_t size = mBinaryReader->ReadULong();
        for(uint64_t i = 0; i < size; ++i)
        {
            T value;
            Xfer("", value);
            set.insert(value);
        }
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteULong(set.size());
        for(auto& entry : set)
        {
            // A little tricky - iterating elements in a set is always const (changing set elements breaks the set).
            // But we "know" that we are writing here, so not going to write the set, only read it. So, const cast to the rescue.
            Xfer("", const_cast<T&>(entry));
        }
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        //TODO
    }
}

template<typename T>
inline void PersistState::Xfer(const char* name, std::unordered_map<std::string, T>& map)
{
    if(mBinaryReader != nullptr)
    {
        map.clear();
        uint64_t size = mBinaryReader->ReadULong();
        for(uint64_t i = 0; i < size; ++i)
        {
            std::string key = mBinaryReader->ReadString32();
            T value;
            Xfer("", value);
            map[key] = value;
        }
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteULong(map.size());
        for(auto& entry : map)
        {
            mBinaryWriter->WriteMedString(entry.first);
            Xfer("", entry.second);
        }
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        //TODO
    }
}

template<typename T>
inline void PersistState::Xfer(const char* name, std::string_map_ci<T>& map)
{
    if(mBinaryReader != nullptr)
    {
        map.clear();
        uint64_t size = mBinaryReader->ReadULong();
        for(uint64_t i = 0; i < size; ++i)
        {
            std::string key = mBinaryReader->ReadString32();
            T value;
            Xfer("", value);
            map[key] = value;
        }
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteULong(map.size());
        for(auto& entry : map)
        {
            mBinaryWriter->WriteMedString(entry.first);
            Xfer("", entry.second);
        }
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        //TODO
    }
}