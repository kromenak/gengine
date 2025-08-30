//
// Clark Kromenaker
//
// A class used for saving/loading data to/from a save file on disk.
// The "Xfer" functions are used to transfer the data.
//
#pragma once
#include <bitset>
#include <cstdint>
#include <set>
#include <type_traits> // std::enable_if
#include <utility> // std::declval

#include <string>
#include <unordered_map>

#include "AssetManager.h"
#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "StringUtil.h" // for string maps

class IniParser;
class IniWriter;

class Circle;
class LineSegment;
class Matrix4;
class Quaternion;
class Rect;
struct UIGrid;
struct UIHexagram;
struct UIRectangle;
class Vector2;
class Vector3;

#define PERSIST_VAR(var) #var, var

// These are C++ template "traits" that enable some fairly complex template metaprogramming shenanigans/black magic.
// Basically, for a type T, this will evaluate to "true" if the type has an OnPersist(PersistState&) function and "false" if not.
template <typename T, typename ArgType, typename = void>
struct HasOnPersistFunction : std::false_type { };
template <typename T, typename ArgType>
struct HasOnPersistFunction<T, ArgType, std::void_t<decltype(std::declval<T>().OnPersist(std::declval<ArgType>()))>> : std::true_type { };

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

    PersistState(PersistState& other) = delete;
    PersistState& operator=(const PersistState& other) = delete;

    PersistState(PersistState&& other) = default;
    PersistState& operator=(PersistState&& other) = default;

    bool IsSaving() const { return mMode == PersistMode::Save; }
    bool IsLoading() const { return mMode == PersistMode::Load; }

    // Version
    void SetFormatVersionNumber(int versionNumber) { mFormatVersionNumber = versionNumber; }
    int GetFormatVersionNumber() { return mFormatVersionNumber; }

    // Bytes
    void Xfer(const char* name, uint8_t* bytes, size_t bytesSize);

    // Strings
    void Xfer(const char* name, char* value, size_t valueSize);
    void Xfer(const char* name, std::string& value);

    // Bool
    void Xfer(const char* name, bool& value);

    // Integers
    //TODO: int8_t/uint8_t

    void Xfer(const char* name, int16_t& value);
    void Xfer(const char* name, uint16_t& value);

    void Xfer(const char* name, int32_t& value);
    void Xfer(const char* name, uint32_t& value);

    void Xfer(const char* name, int64_t& value);
    void Xfer(const char* name, uint64_t& value);

    // Floats
    void Xfer(const char* name, float& value);
    //TODO: Double?

    // Math Types
    void Xfer(const char* name, Vector2& value);
    void Xfer(const char* name, Vector3& value);
    void Xfer(const char* name, Quaternion& value);
    void Xfer(const char* name, Matrix4& value);

    // Primitives
    void Xfer(const char* name, LineSegment& value);
    void Xfer(const char* name, Circle& value);
    void Xfer(const char* name, Rect& value);

    // UI Primitives
    void Xfer(const char* name, UIRectangle& value);
    void Xfer(const char* name, UIGrid& value);
    void Xfer(const char* name, UIHexagram& value);

    // Collections
    template<typename T> void Xfer(const char* name, std::vector<T>& vector, bool loadInPlace = false);
    template<typename T> void Xfer(const char* name, std::set<T>& set);
    template<typename T> void Xfer(const char* name, std::unordered_set<T>& set);
    template<typename T> void Xfer(const char* name, std::unordered_map<std::string, T>& map);
    template<typename T> void Xfer(const char* name, std::string_map_ci<T>& map);
    template<int T> void Xfer(const char* name, std::bitset<T>& bitset);
    template<typename T, typename U> void Xfer(const char* name, std::pair<T, U>& pair);
    void Xfer(const char* name, std::string_set_ci& set);

    // Conversion (Xfer T as U)
    template<typename T, typename U> void Xfer(const char* name, T& value)
    {
        U uValue = static_cast<U>(value);
        Xfer(name, uValue);
        value = static_cast<T>(uValue);
    }

    // Generic Fallback
    // The complex "enable_if" bit ensures that this function is only used for types T that have an OnPersist(PersistState&) member function.
    template<typename T>
    typename std::enable_if<HasOnPersistFunction<T, PersistState&>::value>::type Xfer(const char* name, T& obj)
    {
        obj.OnPersist(*this);
    }

    // This version is used if no OnPersist function is detected. Primarily meant for Assets right now!
    template<typename T> void Xfer(const char* name, T*& asset);

    // Helpers
    BinaryReader* GetBinaryReader() const { return mBinaryReader; }
    BinaryWriter* GetBinaryWriter() const { return mBinaryWriter; }

private:
    PersistFormat mFormat = PersistFormat::Text;
    PersistMode mMode = PersistMode::Save;

    BinaryReader* mBinaryReader = nullptr;
    IniParser* mIniReader = nullptr;

    BinaryWriter* mBinaryWriter = nullptr;
    IniWriter* mIniWriter = nullptr;

    int mFormatVersionNumber = 1;
};

template<typename T>
inline void PersistState::Xfer(const char* name, std::vector<T>& vector, bool loadInPlace)
{
    if(mBinaryReader != nullptr)
    {
        if(!loadInPlace)
        {
            vector.clear();
        }

        uint64_t size = mBinaryReader->ReadULong();
        for(uint64_t i = 0; i < size; ++i)
        {
            if(i < vector.size())
            {
                Xfer("", vector[static_cast<size_t>(i)]);
            }
            else
            {
                // The braces ensure "uniform initialization" - the value T is always initialized.
                // Most important for pointers, so that an empty pointer is set to nullptr instead of garbage.
                T value { };
                Xfer("", value);
                vector.push_back(value);
            }
        }
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteULong(vector.size());
        for(auto& entry : vector)
        {
            Xfer("", entry);
        }
    }
}

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
}

template<typename T>
inline void PersistState::Xfer(const char* name, std::unordered_set<T>& set)
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
}

template<int T>
inline void PersistState::Xfer(const char* name, std::bitset<T>& bitset)
{
    if(T <= sizeof(unsigned long long))
    {
        if(mBinaryReader != nullptr)
        {
            bitset = std::bitset<T>(mBinaryReader->ReadULong());
        }
        else if(mBinaryWriter != nullptr)
        {
            mBinaryWriter->WriteULong(bitset.to_ullong());
        }
    }
    else
    {
        if(mBinaryReader != nullptr)
        {
            bitset = std::bitset<T>(mBinaryReader->ReadString(T));
        }
        else if(mBinaryWriter != nullptr)
        {
            mBinaryWriter->WriteString(bitset.to_string());
        }
    }
}

template<typename T, typename U>
inline void PersistState::Xfer(const char* name, std::pair<T, U>& pair)
{
    Xfer("", pair.first);
    Xfer("", pair.second);
}

template<typename T>
inline void PersistState::Xfer(const char* name, T*& asset)
{
    std::string assetName = asset != nullptr ? asset->GetName() : "";
    AssetScope assetScope = asset != nullptr ? asset->GetScope() : AssetScope::Global;

    // Either read in or write out the asset name and scope.
    Xfer("", assetName);
    Xfer<AssetScope, int>("", assetScope);

    // If loading, resolve the loaded name/scope to an actual asset, if at all possible.
    if(IsLoading())
    {
        if(!assetName.empty())
        {
            asset = gAssetManager.GetOrLoadAsset<T>(assetName, assetScope);
        }
        else
        {
            asset = nullptr;
        }
    }
}