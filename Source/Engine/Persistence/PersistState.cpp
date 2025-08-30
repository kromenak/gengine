#include "PersistState.h"

#include "IniParser.h"
#include "IniWriter.h"

#include "Circle.h"
#include "LineSegment.h"
#include "Rect.h"
#include "UIGrids.h"
#include "UIHexagrams.h"
#include "UIRectangles.h"

PersistState::PersistState(const char* filePath, PersistFormat format, PersistMode mode) :
    mFormat(format),
    mMode(mode)
{
    if(mode == PersistMode::Save)
    {
        if(format == PersistFormat::Text)
        {
            mIniWriter = new IniWriter(filePath);
        }
        else if(format == PersistFormat::Binary)
        {
            mBinaryWriter = new BinaryWriter(filePath);
        }
    }
    else if(mode == PersistMode::Load)
    {
        if(format == PersistFormat::Text)
        {
            mIniReader = new IniParser(filePath);
        }
        else if(format == PersistFormat::Binary)
        {
            mBinaryReader = new BinaryReader(filePath);
        }
    }
}

PersistState::~PersistState()
{
    delete mBinaryReader;
    delete mBinaryWriter;
    delete mIniReader;
    delete mIniWriter;
}

void PersistState::Xfer(const char* name, uint8_t* bytes, size_t bytesSize)
{
    if(mBinaryReader != nullptr)
    {
        mBinaryReader->Read(bytes, bytesSize);
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->Write(bytes, bytesSize);
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

void PersistState::Xfer(const char* name, char* value, size_t valueSize)
{
    if(mBinaryReader != nullptr)
    {
        mBinaryReader->Read(value, valueSize);
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->Write(value, valueSize);
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

void PersistState::Xfer(const char* name, std::string& value)
{
    if(mBinaryReader != nullptr)
    {
        value = mBinaryReader->ReadString32();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteMedString(value);
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        mIniWriter->WriteKeyValue(name, value.c_str());
    }
}

void PersistState::Xfer(const char* name, bool& value)
{
    if(mBinaryReader != nullptr)
    {
        value = mBinaryReader->ReadByte() != 0;
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteByte(value ? 1 : 0);
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        mIniWriter->WriteKeyValue(name, value ? "true" : "false");
    }
}

void PersistState::Xfer(const char* name, int16_t& value)
{
    if(mBinaryReader != nullptr)
    {
        value = mBinaryReader->ReadShort();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteShort(value);
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        mIniWriter->WriteKeyValue(name, std::to_string(value).c_str());
    }
}

void PersistState::Xfer(const char* name, uint16_t& value)
{
    if(mBinaryReader != nullptr)
    {
        value = mBinaryReader->ReadUShort();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteUShort(value);
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        mIniWriter->WriteKeyValue(name, std::to_string(value).c_str());
    }
}

void PersistState::Xfer(const char* name, int32_t& value)
{
    if(mBinaryReader != nullptr)
    {
        value = mBinaryReader->ReadInt();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteInt(value);
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        mIniWriter->WriteKeyValue(name, std::to_string(value).c_str());
    }
}

void PersistState::Xfer(const char* name, uint32_t& value)
{
    if(mBinaryReader != nullptr)
    {
        value = mBinaryReader->ReadUInt();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteUInt(value);
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        mIniWriter->WriteKeyValue(name, std::to_string(value).c_str());
    }
}

void PersistState::Xfer(const char* name, int64_t& value)
{
    if(mBinaryReader != nullptr)
    {
        value = mBinaryReader->ReadLong();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteLong(value);
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

void PersistState::Xfer(const char* name, uint64_t& value)
{
    if(mBinaryReader != nullptr)
    {
        value = mBinaryReader->ReadULong();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteULong(value);
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

void PersistState::Xfer(const char* name, float& value)
{
    if(mBinaryReader != nullptr)
    {
        value = mBinaryReader->ReadFloat();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteFloat(value);
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        mIniWriter->WriteKeyValue(name, std::to_string(value).c_str());
    }
}

void PersistState::Xfer(const char* name, Vector2& value)
{
    if(mBinaryReader != nullptr)
    {
        value.x = mBinaryReader->ReadFloat();
        value.y = mBinaryReader->ReadFloat();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteFloat(value.x);
        mBinaryWriter->WriteFloat(value.y);
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

void PersistState::Xfer(const char* name, Vector3& value)
{
    if(mBinaryReader != nullptr)
    {
        value.x = mBinaryReader->ReadFloat();
        value.y = mBinaryReader->ReadFloat();
        value.z = mBinaryReader->ReadFloat();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteFloat(value.x);
        mBinaryWriter->WriteFloat(value.y);
        mBinaryWriter->WriteFloat(value.z);
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

void PersistState::Xfer(const char* name, Quaternion& value)
{
    if(mBinaryReader != nullptr)
    {
        value.x = mBinaryReader->ReadFloat();
        value.y = mBinaryReader->ReadFloat();
        value.z = mBinaryReader->ReadFloat();
        value.w = mBinaryReader->ReadFloat();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteFloat(value.x);
        mBinaryWriter->WriteFloat(value.y);
        mBinaryWriter->WriteFloat(value.z);
        mBinaryWriter->WriteFloat(value.w);
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

void PersistState::Xfer(const char* name, Matrix4& value)
{
    float* vals = value;
    if(mBinaryReader != nullptr)
    {
        for(int i = 0; i < 16; ++i)
        {
            vals[i] = mBinaryReader->ReadFloat();
        }
    }
    else if(mBinaryWriter != nullptr)
    {
        for(int i = 0; i < 16; ++i)
        {
            mBinaryWriter->WriteFloat(vals[i]);
        }
    }
}

void PersistState::Xfer(const char* name, LineSegment& value)
{
    if(mBinaryReader != nullptr)
    {
        value.start.x = mBinaryReader->ReadFloat();
        value.start.y = mBinaryReader->ReadFloat();
        value.start.z = mBinaryReader->ReadFloat();

        value.end.x = mBinaryReader->ReadFloat();
        value.end.y = mBinaryReader->ReadFloat();
        value.end.z = mBinaryReader->ReadFloat();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteFloat(value.start.x);
        mBinaryWriter->WriteFloat(value.start.y);
        mBinaryWriter->WriteFloat(value.start.z);

        mBinaryWriter->WriteFloat(value.end.x);
        mBinaryWriter->WriteFloat(value.end.y);
        mBinaryWriter->WriteFloat(value.end.z);
    }
}

void PersistState::Xfer(const char* name, Circle& value)
{
    if(mBinaryReader != nullptr)
    {
        value.center.x = mBinaryReader->ReadFloat();
        value.center.y = mBinaryReader->ReadFloat();
        value.radius = mBinaryReader->ReadFloat();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteFloat(value.center.x);
        mBinaryWriter->WriteFloat(value.center.y);
        mBinaryWriter->WriteFloat(value.radius);
    }
}

void PersistState::Xfer(const char* name, Rect& value)
{
    if(mBinaryReader != nullptr)
    {
        value.x = mBinaryReader->ReadFloat();
        value.y = mBinaryReader->ReadFloat();
        value.width = mBinaryReader->ReadFloat();
        value.height = mBinaryReader->ReadFloat();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteFloat(value.x);
        mBinaryWriter->WriteFloat(value.y);
        mBinaryWriter->WriteFloat(value.width);
        mBinaryWriter->WriteFloat(value.height);
    }
}

void PersistState::Xfer(const char* name, UIRectangle& value)
{
    if(mBinaryReader != nullptr)
    {
        value.center.x = mBinaryReader->ReadFloat();
        value.center.y = mBinaryReader->ReadFloat();
        value.size.x = mBinaryReader->ReadFloat();
        value.size.y = mBinaryReader->ReadFloat();
        value.angle = mBinaryReader->ReadFloat();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteFloat(value.center.x);
        mBinaryWriter->WriteFloat(value.center.y);
        mBinaryWriter->WriteFloat(value.size.x);
        mBinaryWriter->WriteFloat(value.size.y);
        mBinaryWriter->WriteFloat(value.angle);
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

void PersistState::Xfer(const char* name, UIGrid& value)
{
    if(mBinaryReader != nullptr)
    {
        value.center.x = mBinaryReader->ReadFloat();
        value.center.y = mBinaryReader->ReadFloat();
        value.size.x = mBinaryReader->ReadFloat();
        value.size.y = mBinaryReader->ReadFloat();
        value.angle = mBinaryReader->ReadFloat();
        value.divisions = mBinaryReader->ReadInt();
        value.drawBorder = mBinaryReader->ReadByte() == 1;
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteFloat(value.center.x);
        mBinaryWriter->WriteFloat(value.center.y);
        mBinaryWriter->WriteFloat(value.size.x);
        mBinaryWriter->WriteFloat(value.size.y);
        mBinaryWriter->WriteFloat(value.angle);
        mBinaryWriter->WriteInt(value.divisions);
        mBinaryWriter->WriteByte(value.drawBorder ? 1 : 0);
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

void PersistState::Xfer(const char* name, UIHexagram& value)
{
    if(mBinaryReader != nullptr)
    {
        value.center.x = mBinaryReader->ReadFloat();
        value.center.y = mBinaryReader->ReadFloat();
        value.radius = mBinaryReader->ReadFloat();
        value.angle = mBinaryReader->ReadFloat();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteFloat(value.center.x);
        mBinaryWriter->WriteFloat(value.center.y);
        mBinaryWriter->WriteFloat(value.radius);
        mBinaryWriter->WriteFloat(value.angle);
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

void PersistState::Xfer(const char* name, std::string_set_ci& set)
{
    if(mBinaryReader != nullptr)
    {
        set.clear();
        uint64_t size = mBinaryReader->ReadULong();
        for(uint64_t i = 0; i < size; ++i)
        {
            std::string value;
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
            Xfer("", const_cast<std::string&>(entry));
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