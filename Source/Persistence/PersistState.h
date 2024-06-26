//
// Clark Kromenaker
//
// A class used for saving/loading data to/from a save file on disk.
// The "Xfer" functions are used to transfer the data.
//
#pragma once
#include <cstdint>
#include <string>

class BinaryReader;
class BinaryWriter;
class IniParser;
class IniWriter;

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

    void Xfer(const char* name, char* value, size_t valueSize);
    void Xfer(const char* name, std::string& value);

    void Xfer(const char* name, int32_t& value);
    void Xfer(const char* name, uint32_t& value);

    void Xfer(const char* name, int16_t& value);
    void Xfer(const char* name, uint16_t& value);
    
private:
    PersistMode mMode = PersistMode::Save;
    PersistFormat mFormat = PersistFormat::Text;

    BinaryReader* mBinaryReader = nullptr;
    IniParser* mIniReader = nullptr;

    BinaryWriter* mBinaryWriter = nullptr;
    IniWriter* mIniWriter = nullptr;
};