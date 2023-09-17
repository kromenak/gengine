//
// Clark Kromenaker
//
// The "graphics API" (GAPI) provides a uniform interface to the rest of the game
// for performing various graphics tasks.
//
// This allows the game to use different graphics libraries while isolating the graphics code.
//
#pragma once
#include "Color32.h"
#include "MeshDefinition.h"
#include "Texture.h" // For WrapMode/FilterMode
#include "VertexDefinition.h"

typedef void* TextureHandle;
typedef void* BufferHandle;

class GAPI
{
public:
    template<typename T>
    static void Set()
    {
        // Shutdown the current GAPI.
        if(sCurrent != nullptr)
        {
            sCurrent->Shutdown();
            delete sCurrent;
            sCurrent = nullptr;
        }

        // Create the new GAPI.
        sCurrent = new T();
        sCurrent->Init();
    }

    static GAPI* Get() { return sCurrent; }

private:
    static GAPI* sCurrent;

public:
    virtual ~GAPI() { }

    // Init and shutdown
    virtual bool Init() = 0;
    virtual void Shutdown() = 0;

     // IMGUI Support
    virtual void ImGuiNewFrame() = 0;
    virtual void ImGuiRenderDrawData() = 0;

    // Render loop
    virtual void Clear(Color32 clearColor) = 0;
    virtual void Present() = 0;

    // Polygon settings
    enum class CullMode
    {
        None,
        Back,
        Front,
        All
    };
    enum class WindingOrder
    {
        CounterClockwise,
        Clockwise,
    };
    enum class FillMode
    {
        Wireframe,
        Filled
    };
    virtual void SetPolygonCullMode(CullMode cullMode) = 0;
    virtual void SetPolygonWindingOrder(WindingOrder windingOrder) = 0;
    virtual void SetPolygonFillMode(FillMode fillMode) = 0;

    // Coordinate space settings
    enum class Handedness
    {
        LeftHand,
        RightHand
    };
    virtual void SetViewSpaceHandedness(Handedness handedness) = 0;

    // Viewport
    virtual void SetViewport(int32_t x, int32_t y, uint32_t width, uint32_t height) = 0;

    // Depth Buffer
    virtual void SetDepthWriteEnabled(bool enabled) = 0;
    virtual void SetDepthTestEnabled(bool enabled) = 0;

    // Blending
    enum class BlendMode
    {
        AlphaBlend, // Blend src into dst using src's alpha channel
        Multiply    // Multiply pixels of src by pixels of dst
    };
    virtual void SetBlendEnabled(bool enabled) = 0;
    virtual void SetBlendMode(BlendMode blendMode) = 0;

    // Textures
    virtual TextureHandle CreateTexture(uint32_t width, uint32_t height, uint8_t* pixels = nullptr) = 0;
    virtual void DestroyTexture(TextureHandle handle) = 0;

    virtual void SetTexturePixels(TextureHandle handle, uint32_t width, uint32_t height, uint8_t* pixels) = 0;
    virtual void GenerateMipmaps(TextureHandle handle) = 0;
    virtual void SetTextureWrapMode(TextureHandle handle, Texture::WrapMode wrapMode) = 0;
    virtual void SetTextureFilterMode(TextureHandle handle, Texture::FilterMode filterMode, bool useMipmaps) = 0;

    virtual void ActivateTexture(TextureHandle handle, uint8_t textureUnit = 0) = 0;

    // Cubemaps
    struct CubemapSide
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint8_t* pixels = nullptr;
    };
    struct CubemapParams
    {
        CubemapSide left;
        CubemapSide right;
        CubemapSide back;
        CubemapSide front;
        CubemapSide bottom;
        CubemapSide top;
    };
    virtual TextureHandle CreateCubemap(const CubemapParams& params) = 0;
    virtual void DestroyCubemap(TextureHandle handle) = 0;
    virtual void ActivateCubemap(TextureHandle handle) = 0;
    
    // Vertex & Index Buffers
    virtual BufferHandle CreateVertexBuffer(uint32_t vertexCount, const VertexDefinition& vertexDefinition, void* data = nullptr, MeshUsage usage = MeshUsage::Static) = 0;
    virtual void DestroyVertexBuffer(BufferHandle handle) = 0;
    virtual void SetVertexBufferData(BufferHandle handle, uint32_t offset, uint32_t size, void* data) = 0;

    virtual BufferHandle CreateIndexBuffer(uint32_t indexCount, uint16_t* indexData = nullptr, MeshUsage usage = MeshUsage::Static) = 0;
    virtual void DestroyIndexBuffer(BufferHandle handle) = 0;
    virtual void SetIndexBufferData(BufferHandle handle, uint32_t indexCount, uint16_t* indexData) = 0;

    // Drawing
    enum class Primitive
    {
        Lines,
        Triangles,
        TriangleStrip,
        TriangleFan
    };
    virtual void Draw(Primitive primitive, BufferHandle vertexBuffer) = 0;
    virtual void Draw(Primitive primitive, BufferHandle vertexBuffer, uint32_t vertexOffset, uint32_t vertexCount) = 0;
    virtual void Draw(Primitive primitive, BufferHandle vertexBuffer, BufferHandle indexBuffer) = 0;
    virtual void Draw(Primitive primitive, BufferHandle vertexBuffer, BufferHandle indexBuffer, uint32_t indexOffset, uint32_t indexCount) = 0;
};