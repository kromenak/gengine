//
// Clark Kromenaker
//
// A codec for encoding to PNG or decoding from PNG.
//
#pragma once
#include <cstdint>

class BinaryReader;
class BinaryWriter;

namespace PNG
{
    enum class CodecResult
    {
        Success,
        Error
    };

    // When encoding, provides the input image data to the encoder.
    // When decoding, provides the decoded image data.
    struct ImageData
    {
        // Width and height of the image.
        uint32_t width = 0;
        uint32_t height = 0;

        // The number of bytes per pixel in the pixel data.
        // Common/expected values here could be 3 for RGB or 4 for RGBA.
        int bytesPerPixel = 0;

        // Uncompressed RGBA pixel data.
        // On encode, this data is encoded to PNG. On decode, the decoded data is returned here.
        // The caller always owns this data and is responsible for it.
        uint8_t* pixelData = nullptr;
    };

    // Encode to file, byte array, or binary writer.
    CodecResult Encode(const ImageData& input, const char* filePath);
    CodecResult Encode(const ImageData& input, uint8_t* pngData, uint32_t pngDataBufferSize, uint32_t& outPngDataLength);
    CodecResult Encode(const ImageData& input, BinaryWriter& writer);

    // Decode from file, byte array, or binary reader.
    CodecResult Decode(const char* filePath, ImageData& result);
    CodecResult Decode(uint8_t* pngData, uint32_t pngDataLength, ImageData& result);
    CodecResult Decode(BinaryReader& reader, ImageData& result);
}