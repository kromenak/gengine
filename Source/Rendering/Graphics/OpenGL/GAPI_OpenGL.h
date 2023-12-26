#pragma once
#include "GAPI.h"

#include <SDL.h>

class GAPI_OpenGL : public GAPI
{
public:
    bool Init() override;
    void Shutdown() override;

    void ImGuiNewFrame() override;
    void ImGuiRenderDrawData() override;

    void Clear(Color32 clearColor) override;
    void Present() override;

    void SetPolygonCullMode(CullMode cullMode) override;
    void SetPolygonWindingOrder(WindingOrder windingOrder) override;
    void SetPolygonFillMode(FillMode fillMode) override;

    void SetViewSpaceHandedness(Handedness handedness) override;

    void SetViewport(int32_t x, int32_t y, uint32_t width, uint32_t height) override;
    void SetScissorRect(bool enabled, const Rect& rect) override;

    void SetDepthWriteEnabled(bool enabled) override;
    void SetDepthTestEnabled(bool enabled) override;

    void SetBlendEnabled(bool enabled) override;
    void SetBlendMode(BlendMode blendMode) override;

    TextureHandle CreateTexture(uint32_t width, uint32_t height, uint8_t* pixels) override;
    void DestroyTexture(TextureHandle handle) override;
    void SetTexturePixels(TextureHandle handle, uint32_t width, uint32_t height, uint8_t* pixels) override;
    void GenerateMipmaps(TextureHandle handle) override;
    void SetTextureWrapMode(TextureHandle handle, Texture::WrapMode wrapMode) override;
    void SetTextureFilterMode(TextureHandle handle, Texture::FilterMode filterMode, bool useMipmaps) override;
    void ActivateTexture(TextureHandle handle, uint8_t textureUnit) override;

    TextureHandle CreateCubemap(const CubemapParams& params) override;
    void DestroyCubemap(TextureHandle handle) override;
    void ActivateCubemap(TextureHandle handle) override;

    BufferHandle CreateVertexBuffer(uint32_t vertexCount, const VertexDefinition& vertexDefinition, void* data, MeshUsage usage) override;
    void DestroyVertexBuffer(BufferHandle handle) override;
    void SetVertexBufferData(BufferHandle handle, uint32_t offset, uint32_t size, void* data) override;

    BufferHandle CreateIndexBuffer(uint32_t indexCount, uint16_t* indexData, MeshUsage usage) override;
    void DestroyIndexBuffer(BufferHandle handle) override;
    void SetIndexBufferData(BufferHandle handle, uint32_t indexCount, uint16_t* indexData) override;

    ShaderHandle CreateShader(const uint8_t* vertSource, const uint8_t* fragSource) override;
    void DestroyShader(ShaderHandle handle) override;
    void ActivateShader(ShaderHandle handle) override;

    void SetShaderUniformInt(ShaderHandle handle, const char* name, int value) override;
    void SetShaderUniformFloat(ShaderHandle handle, const char* name, float value) override;
    void SetShaderUniformVector3(ShaderHandle handle, const char* name, const Vector3& value) override;
    void SetShaderUniformVector4(ShaderHandle handle, const char* name, const Vector4& value) override;
    void SetShaderUniformMatrix4(ShaderHandle handle, const char* name, const Matrix4& mat) override;
    void SetShaderUniformColor(ShaderHandle handle, const char* name, const Color32& color) override;

    void Draw(Primitive primitive, BufferHandle vertexBuffer) override;
    void Draw(Primitive primitive, BufferHandle vertexBuffer, uint32_t vertexOffset, uint32_t vertexCount) override;
    void Draw(Primitive primitive, BufferHandle vertexBuffer, BufferHandle indexBuffer) override;
    void Draw(Primitive primitive, BufferHandle vertexBuffer, BufferHandle indexBuffer, uint32_t indexOffset, uint32_t indexCount) override;

private:
    // Context handle for rendering in OpenGL.
    void* mContext = nullptr;
};