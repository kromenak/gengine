#include "PNGCodec.h"

#include <cstdio>
#include <cstdlib>
#include <memory>

#include <png.h>

#include "BinaryReader.h"
#include "BinaryWriter.h"

using namespace PNG;

namespace
{
    void OnPNGError(png_structp pngContext, png_const_charp message)
    {
        //png_get_error_ptr(pngContext));
        printf("PNG error: %s\n", message);
    }

    void OnPNGWarning(png_structp pngContext, png_const_charp message)
    {
        //png_get_error_ptr(pngContext));
        printf("PNG warning: %s\n", message);
    }

    void OnPNGReadData(png_structp pngRead, png_bytep data, size_t length)
    {
        // Cast io ptr to the binary reader being used to read PNG data.
        BinaryReader* reader = static_cast<BinaryReader*>(png_get_io_ptr(pngRead));
        if(reader == nullptr)
        {
            png_error(pngRead, "Binary reader is null!");
        }

        // Read the data, populating the provided data field.
        uint32_t readBytes = reader->Read(data, length);
        if(readBytes != length)
        {
            png_error(pngRead, "Unexpected EOF!");
        }
    }

    void OnPNGReadStatusUpdated(png_structp pngRead, png_uint_32 nextRow, int nextPass)
    {
        // This *could* be used to track PNG read status...but no need right now.
        //printf("Read PNG row %d (pass %d)\n", row - 1, pass - 1);
    }

    void OnPNGWriteData(png_structp pngWrite, png_bytep data, size_t length)
    {
        // Cast io ptr to the binary writer being used to write PNG data.
        BinaryWriter* writer = static_cast<BinaryWriter*>(png_get_io_ptr(pngWrite));
        if(writer == nullptr)
        {
            png_error(pngWrite, "Binary writer is null!");
        }

        // Write the data.
        writer->Write(data, length);
    }

    void OnPNGWriteFlush(png_structp pngWrite)
    {
        // Cast io ptr to the binary writer being used to write PNG data.
        BinaryWriter* writer = static_cast<BinaryWriter*>(png_get_io_ptr(pngWrite));
        if(writer == nullptr)
        {
            png_error(pngWrite, "Binary writer is null!");
        }

        // Flush the writer.
        writer->Flush();
    }

    void OnPNGWriteStatusUpdated(png_structp pngWrite, png_uint_32 nextRow, int nextPass)
    {
        // This *could* be used to track PNG write status...but no need right now.
        //printf("Write PNG row %d (pass %d)\n", row - 1, pass - 1);
    }

    std::unique_ptr<png_bytep[]> AllocAndInitRowPointers(uint32_t rowCount, uint8_t* pixelData, size_t bytesPerRow)
    {
        // Neither the encoder nor the decoder function on the pixel byte array directly.
        // Instead, you need to pass an array of pointers INTO the pixel byte array. Each pointer is the start of a row of pixels in the array.
        std::unique_ptr<png_bytep[]> rowPointers = std::make_unique<png_bytep[]>(rowCount);
        for(int row = 0; row < rowCount; ++row)
        {
            // Since we know the bytes per row, as well as which row we're on...
            // ...we can calculate the byte offset for each row and save it in the array of row pointers.
            rowPointers[row] = pixelData + row * bytesPerRow;
        }
        return rowPointers;
    }
}

CodecResult PNG::Encode(const ImageData& input, const char* filePath)
{
    BinaryWriter writer(filePath);
    return Encode(input, writer);
}

CodecResult PNG::Encode(const ImageData& input, uint8_t* pngData, uint32_t pngDataBufferSize, uint32_t& outPngDataLength)
{
    BinaryWriter writer(pngData, pngDataBufferSize);
    CodecResult result = Encode(input, writer);
    outPngDataLength = writer.GetPosition();
    return result;
}

CodecResult PNG::Encode(const ImageData& input, BinaryWriter& writer)
{
    // Create PNG write context.
    png_structp pngWrite = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, OnPNGError, OnPNGWarning);
    if(pngWrite == nullptr) { return CodecResult::Error; }

    // Create PNG info struct.
    png_infop pngInfo = png_create_info_struct(pngWrite);
    if(pngInfo == nullptr)
    {
        png_destroy_write_struct(&pngWrite, nullptr);
        return CodecResult::Error;
    }

    // Set up error handling for libpng. This if-statement is called via jump mechanism if an error occurs.
    // It should clean up and return an error.
    if(setjmp(png_jmpbuf(pngWrite)))
    {
        png_destroy_write_struct(&pngWrite, &pngInfo);
        return CodecResult::Error;
    }

    // By default, libpng writes to file. But we need custom write behavior.
    // So, tell libpng to use two custom write functions, passing in our BinaryWriter.
    png_set_write_fn(pngWrite, &writer, OnPNGWriteData, OnPNGWriteFlush);

    // Sets a callback that's called to get write status updates.
    png_set_write_status_fn(pngWrite, OnPNGWriteStatusUpdated);

    // If desired, this would silence a warning about invalid palette indexes on write.
    //png_set_check_for_invalid_index(png_ptr, 0);

    // If we don't want to write out the PNG 8-byte signature, we could call this.
    //png_set_sig_bytes(png_ptr, 8);

    // The docs recommend leaving these as defaults, but they could be tuned for compression vs speed.
    //png_set_filter(...)
    //png_set_compression_level(...)

    // If needed, this inverts the alpha value on write.
    //png_set_invert_alpha(png_ptr);

    // Determine "color type" from bit depth. Assuming either 24-bit RGB or 32-bit RGBA here.
    int colorType = PNG_COLOR_TYPE_RGB;
    if(input.bytesPerPixel == 4)
    {
        colorType = PNG_COLOR_TYPE_RGB_ALPHA;
    }

    // Set IHDR chunk.
    png_set_IHDR(pngWrite, pngInfo, input.width, input.height, 8, colorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    // Everything is configured - start by writing info chunks.
    png_write_info(pngWrite, pngInfo);

    //TODO: Plenty of transforms can be specified before writing the data, if needed.

    // Allocate row pointers from the pixel data.
    std::unique_ptr<png_bytep[]> rowPointers = AllocAndInitRowPointers(input.height, input.pixelData, 4 * input.width);

    // Write out the entire image.
    png_write_image(pngWrite, rowPointers.get());

    // Done writing, delete structs and return success.
    png_write_end(pngWrite, pngInfo);
    png_destroy_write_struct(&pngWrite, &pngInfo);
    return CodecResult::Success;
}

CodecResult PNG::Decode(const char* filePath, ImageData& result)
{
    BinaryReader reader(filePath);
    return Decode(reader, result);
}

CodecResult PNG::Decode(uint8_t* pngData, uint32_t pngDataLength, ImageData& result)
{
    BinaryReader reader(pngData, pngDataLength);
    return Decode(reader, result);
}

CodecResult PNG::Decode(BinaryReader& reader, ImageData& result)
{
    // Create PNG read context.
    png_structp pngRead = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, OnPNGError, OnPNGWarning);
    if(pngRead == nullptr) { return CodecResult::Error; }

    // Create PNG info struct.
    png_infop pngInfo = png_create_info_struct(pngRead);
    if(pngInfo == nullptr)
    {
        png_destroy_read_struct(&pngRead, nullptr, nullptr);
        return CodecResult::Error;
    }

    // Set up error handling for libpng. This if-statement is called via jump mechanism if an error occurs.
    // It should clean up and return an error.
    if(setjmp(png_jmpbuf(pngRead)))
    {
        png_destroy_read_struct(&pngRead, &pngInfo, nullptr);
        return CodecResult::Error;
    }

    // The built-in read behavior for libpng is to read from file. But we need to read from memory.
    // So, we'll provide our own read function.
    png_set_read_fn(pngRead, &reader, OnPNGReadData);

    // Use a callback to get progress updates on the PNG operation.
    png_set_read_status_fn(pngRead, OnPNGReadStatusUpdated);

    // Set this if you already read the first 8 bytes to check if this is a PNG file.
    //png_set_sig_bytes(pngRead, 8);

    // Set this if the default compression buffer size (8192) is too big/small for some reason.
    //png_set_compression_buffer_size(pngRead, buffer_size);

    // Set if you want CRC checks to be handled a certain way.
    //png_set_crc_action(pngRead, crit_action, ancil_action);

    // Set if PNG needs to be displayed on-screen, but doesn't look correct. 2.2 is a good default value. Can also be retrieved from the system.
    //png_set_gamma(png_ptr, screen_gamma, output_gamma);

    //TODO: Unsure if this is needed - can be used to ignore chunks that we don't need, which might be more efficient?
    //png_set_keep_unknown_chunks(png_ptr, keep, chunk_list, num_chunks);

    // Read PNG header info.
    png_read_info(pngRead, pngInfo);

    // Parse the IHDR chunk to get image width, height, bit depth, etc.
    int bitDepth = 0;
    int colorType = 0;
    int interlaceType = 0;
    png_get_IHDR(pngRead, pngInfo, &result.width, &result.height, &bitDepth, &colorType, &interlaceType, nullptr, nullptr);

    // Based on color type, determine number of components per pixel. RGB has 3 components, RGBA has components, etc.
    int componentCount = 4;
    if(colorType == PNG_COLOR_TYPE_RGB)
    {
        componentCount = 3;
    }

    // The "bit depth" field returns the number of bits for a single pixel component.
    // To get the bytes per pixel, we need to multiply by the number of components per pixel and divide by number of bits in a byte (8).
    result.bytesPerPixel = (bitDepth * componentCount) / 8;

    //TODO: Gamma?
    //TODO: Other transforms and options can be set here too!

    // We want the decoded image data to be RGBA.
    // This adds an alpha byte during decoding if this is an RGB image.
    png_set_add_alpha(pngRead, 0xff, PNG_FILLER_AFTER);

    // Updates the info struct based on gamma or other config options set since png_read_info.
    png_read_update_info(pngRead, pngInfo);

    // Based on everything configured up to here, we can calculate the number of bytes per row.
    // This should be bytesPerPixel * imageWidth.
    size_t bytesPerRow = png_get_rowbytes(pngRead, pngInfo);

    // Allocate a big block of pixel data for the decoded pixel data.
    result.pixelData = new uint8_t[bytesPerRow * result.height];

    // Allocate row pointers from the pixel data.
    std::unique_ptr<png_bytep[]> rowPointers = AllocAndInitRowPointers(result.height, result.pixelData, bytesPerRow);

    // Read the image data into the row pointers. This fills the pixel data buffer.
    // This is the most expensive part, does the actual decoding.
    png_read_image(pngRead, rowPointers.get());

    // Done reading, delete structs and return success.
    png_read_end(pngRead, pngInfo);
    png_destroy_read_struct(&pngRead, &pngInfo, nullptr);
    return CodecResult::Success;
}