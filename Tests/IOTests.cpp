//
// Clark Kromenaker
//
// Tests for input/output code.
//
#include "catch.hh"

#include "BinaryReader.h"
#include "BinaryWriter.h"

TEST_CASE("Read/Write binary memory works")
{
    char memory[256];

    // Create writer, test default state.
    BinaryWriter writer(memory, 256);
    REQUIRE(writer.OK());
    REQUIRE(writer.GetPosition() == 0);

    // Write a bunch of data.
    writer.WriteDouble(25.25);
    REQUIRE(writer.GetPosition() == 8);

    writer.WriteFloat(3.14f);
    REQUIRE(writer.GetPosition() == 12);

    writer.WriteInt(-42);
    REQUIRE(writer.GetPosition() == 16);

    writer.WriteSByte(-100);
    REQUIRE(writer.GetPosition() == 17);

    writer.WriteShort(-1000);
    REQUIRE(writer.GetPosition() == 19);

    writer.WriteByte(128);
    REQUIRE(writer.GetPosition() == 20);

    writer.WriteUShort(1024);
    REQUIRE(writer.GetPosition() == 22);

    writer.WriteUInt(8675309);
    REQUIRE(writer.GetPosition() == 26);

    const char* textIn = "Hello";
    writer.Write(textIn, strlen(textIn));
    REQUIRE(writer.GetPosition() == 31);

    std::string strIn = "This is a test!";
    writer.WriteTinyString(strIn);
    REQUIRE(writer.GetPosition() == 47);

    std::string strBufferIn = "Smaller than buffer";
    writer.WriteStringBuffer(strBufferIn, 32);
    REQUIRE(writer.GetPosition() == 79);

    // Check that we can seek & skip.
    writer.Seek(0);
    REQUIRE(writer.GetPosition() == 0);

    writer.Skip(100);
    REQUIRE(writer.GetPosition() == 100);

    // Create reader, test default state.
    BinaryReader reader(memory, 256);
    REQUIRE(reader.OK());
    REQUIRE(reader.GetPosition() == 0);

    // Read in, making sure the data is right.
    double d = reader.ReadDouble();
    REQUIRE(d == 25.25);
    REQUIRE(reader.GetPosition() == 8);

    float f = reader.ReadFloat();
    REQUIRE(f == 3.14f);
    REQUIRE(reader.GetPosition() == 12);

    int32_t i = reader.ReadInt();
    REQUIRE(i == -42);
    REQUIRE(reader.GetPosition() == 16);

    int8_t sb = reader.ReadSByte();
    REQUIRE(sb == -100);
    REQUIRE(reader.GetPosition() == 17);

    int16_t s = reader.ReadShort();
    REQUIRE(s == -1000);
    REQUIRE(reader.GetPosition() == 19);

    uint8_t b = reader.ReadByte();
    REQUIRE(b == 128);
    REQUIRE(reader.GetPosition() == 20);

    uint16_t us = reader.ReadUShort();
    REQUIRE(us == 1024);
    REQUIRE(reader.GetPosition() == 22);

    uint32_t ui = reader.ReadUInt();
    REQUIRE(ui == 8675309);
    REQUIRE(reader.GetPosition() == 26);

    char textOut[10] = { 0 };
    reader.Read(textOut, 5);
    REQUIRE(strcmp(textOut, textIn) == 0);
    REQUIRE(reader.GetPosition() == 31);

    std::string strOut = reader.ReadTinyString();
    REQUIRE(strOut == strIn);
    REQUIRE(reader.GetPosition() == 47);

    std::string strBufferOut = reader.ReadStringBuffer(32);
    REQUIRE(strBufferOut == strBufferIn);
    REQUIRE(reader.GetPosition() == 79);

    // Check that we can seek & skip.
    reader.Seek(0);
    REQUIRE(reader.GetPosition() == 0);

    reader.Skip(100);
    REQUIRE(reader.GetPosition() == 100);
}