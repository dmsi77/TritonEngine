// render_context.hpp

#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "object.hpp"
#include "types.hpp"

namespace realware
{
    class cApplication;
    class cTextureAtlasTexture;

    struct sGPUResource
    {
        types::u32 _instance = 0;
        types::u32 _viewInstance = 0;
    };

    struct sBuffer : public sGPUResource
    {
        enum class eType
        {
            NONE = 0,
            VERTEX = 1,
            INDEX = 2,
            UNIFORM = 3,
            LARGE = 4
        };

        eType _type = eType::NONE;
        types::usize _byteSize = 0;
        types::s32 _slot = -1;
    };

    struct sVertexArray : public sGPUResource
    {
    };

    struct sShader : public sGPUResource
    {
        struct sDefinePair
        {
            sDefinePair(const std::string& name, types::usize index) : Name(name), Index(index) {}
            ~sDefinePair() = default;

            std::string Name = "";
            types::usize Index = 0;
        };

        std::string _vertex = "";
        std::string _fragment = "";
    };

    struct sTexture : public sGPUResource
    {
        enum class eType
        {
            NONE = 0,
            TEXTURE_2D = 1,
            TEXTURE_2D_ARRAY = 2
        };

        enum class eFormat
        {
            NONE = 0,
            R8 = 1,
            R8F = 2,
            RGBA8 = 3,
            RGB16F = 4,
            RGBA16F = 5,
            DEPTH_STENCIL = 6,
            RGBA8_MIPS = 7
        };

        types::usize _width = 0;
        types::usize _height = 0;
        types::usize _depth = 0;
        eType _type = eType::NONE;
        eFormat _format = eFormat::NONE;
        types::s32 _slot = -1;
    };

    struct sRenderTarget : public sGPUResource
    {
        std::vector<sTexture*> _colorAttachments = {};
        sTexture* _depthAttachment = nullptr;
    };

    struct sDepthMode
    {
        types::boolean _useDepthTest = types::K_TRUE;
        types::boolean _useDepthWrite = types::K_TRUE;
    };

    struct sBlendMode
    {
        enum class eFactor
        {
            ZERO = 0,
            ONE = 1,
            SRC_COLOR = 2,
            INV_SRC_COLOR = 3,
            SRC_ALPHA = 4,
            INV_SRC_ALPHA = 5
        };

        types::usize _factorCount = 0;
        eFactor _srcFactors[8] = { eFactor::ZERO };
        eFactor _dstFactors[8] = { eFactor::ZERO };
    };

    struct sRenderPass
    {
        struct sDescriptor
        {
            sVertexArray* _vertexArray = nullptr;
            eCategory _inputVertexFormat = eCategory::VERTEX_BUFFER_FORMAT_NONE;
            std::vector<sBuffer*> _inputBuffers = {};
            std::vector<sTexture*> _inputTextures = {};
            std::vector<std::string> _inputTextureNames = {};
            std::vector<cTextureAtlasTexture*> _inputTextureAtlasTextures = {};
            std::vector<std::string> _inputTextureAtlasTextureNames = {};
            sShader* _shader = nullptr;
            sShader* _shaderBase = nullptr;
            eCategory _shaderRenderPath = eCategory::RENDER_PATH_OPAQUE;
            std::string _shaderVertexPath = "";
            std::string _shaderFragmentPath = "";
            std::string _shaderVertexFunc = "";
            std::string _shaderFragmentFunc = "";
            sRenderTarget* _renderTarget = nullptr;
            sDepthMode _depthMode = {};
            sBlendMode _blendMode = {};
            glm::vec4 _viewport = glm::vec4(0.0f);
        };

        sDescriptor _desc;
    };

    class iRenderContext : public iObject
    {
    public:
        explicit iRenderContext(cContext* context) : iObject(context) {}
        virtual ~iRenderContext() = default;

        virtual sBuffer* CreateBuffer(types::usize byteSize, sBuffer::eType type, const void* data) = 0;
        virtual void BindBuffer(const sBuffer* buffer) = 0;
		virtual void BindBufferNotVAO(const sBuffer* buffer) = 0;
        virtual void UnbindBuffer(const sBuffer* buffer) = 0;
        virtual void WriteBuffer(const sBuffer* buffer, types::usize offset, types::usize byteSize, const void* data) = 0;
        virtual void DestroyBuffer(sBuffer* buffer) = 0;
        virtual sVertexArray* CreateVertexArray() = 0;
        virtual void BindVertexArray(const sVertexArray* vertexArray) = 0;
        virtual void BindDefaultVertexArray(const std::vector<sBuffer*>& buffersToBind) = 0;
        virtual void UnbindVertexArray() = 0;
        virtual void DestroyVertexArray(sVertexArray* vertexArray) = 0;
        virtual void BindShader(const sShader* shader) = 0;
        virtual void UnbindShader() = 0;
        virtual sShader* CreateShader(eCategory renderPath, const std::string& vertexPath, const std::string& fragmentPath, const std::vector<sShader::sDefinePair>& definePairs = {}) = 0;
        virtual sShader* CreateShader(const sShader* baseShader, const std::string& vertexFunc, const std::string& fragmentFunc, const std::vector<sShader::sDefinePair>& definePairs = {}) = 0;
        virtual void DefineInShader(sShader* shader, const std::vector<sShader::sDefinePair>& definePairs) = 0;
        virtual void DestroyShader(sShader* shader) = 0;
        virtual void SetShaderUniform(const sShader* shader, const std::string& name, const glm::mat4& matrix) = 0;
        virtual void SetShaderUniform(const sShader* shader, const std::string& name, types::usize count, const types::f32* values) = 0;
        virtual sTexture* CreateTexture(types::usize width, types::usize height, types::usize depth, sTexture::eType type, sTexture::eFormat format, const void* data) = 0;
        virtual sTexture* ResizeTexture(sTexture* texture, const glm::vec2& size) = 0;
        virtual void BindTexture(const sShader* shader, const std::string& name, const sTexture* texture, types::s32 slot) = 0;
        virtual void UnbindTexture(const sTexture* texture) = 0;
        virtual void WriteTexture(const sTexture* texture, const glm::vec3& offset, const glm::vec2& size, const void* data) = 0;
        virtual void WriteTextureToFile(const sTexture* texture, const std::string& filename) = 0;
        virtual void GenerateTextureMips(const sTexture* texture) = 0;
        virtual void DestroyTexture(sTexture* texture) = 0;
        virtual sRenderTarget* CreateRenderTarget(const std::vector<sTexture*>& colorAttachments, sTexture* depthAttachment) = 0;
        virtual void ResizeRenderTargetColors(sRenderTarget* renderTarget, const glm::vec2& size) = 0;
        virtual void ResizeRenderTargetDepth(sRenderTarget* renderTarget, const glm::vec2& size) = 0;
        virtual void UpdateRenderTargetBuffers(sRenderTarget* renderTarget) = 0;
        virtual void BindRenderTarget(const sRenderTarget* renderTarget) = 0;
        virtual void UnbindRenderTarget() = 0;
        virtual void DestroyRenderTarget(sRenderTarget* renderTarget) = 0;
        virtual sRenderPass* CreateRenderPass(const sRenderPass::sDescriptor& descriptor) = 0;
        virtual void BindRenderPass(const sRenderPass* renderPass, sShader* customShader = nullptr) = 0;
        virtual void UnbindRenderPass(const sRenderPass* renderPass) = 0;
        virtual void DestroyRenderPass(sRenderPass* renderPass) = 0;
        virtual void BindDefaultInputLayout() = 0;
        virtual void BindDepthMode(const sDepthMode& blendMode) = 0;
        virtual void BindBlendMode(const sBlendMode& blendMode) = 0;
        virtual void Viewport(const glm::vec4& viewport) = 0;
        virtual void ClearColor(const glm::vec4& color) = 0;
        virtual void ClearDepth(types::f32 depth) = 0;
        virtual void ClearFramebufferColor(types::usize bufferIndex, const glm::vec4& color) = 0;
        virtual void ClearFramebufferDepth(types::f32 depth) = 0;
        virtual void Draw(types::usize indexCount, types::usize vertexOffset, types::usize indexOffset, types::usize instanceCount) = 0;
        virtual void DrawQuad() = 0;
        virtual void DrawQuads(types::usize count) = 0;
    };

    class cOpenGLRenderContext : public iRenderContext
    {
    public:
        cOpenGLRenderContext(cContext* context);
        virtual ~cOpenGLRenderContext() override final;

        inline virtual cType GetType() const override final { return cType("OpenGLRenderContext"); }

        virtual sBuffer* CreateBuffer(types::usize byteSize, sBuffer::eType type, const void* data) override final;
        virtual void BindBuffer(const sBuffer* buffer) override final;
        virtual void BindBufferNotVAO(const sBuffer* buffer) override final;
        virtual void UnbindBuffer(const sBuffer* buffer) override final;
        virtual void WriteBuffer(const sBuffer* buffer, types::usize offset, types::usize byteSize, const void* data) override final;
        virtual void DestroyBuffer(sBuffer* buffer) override final;
        virtual sVertexArray* CreateVertexArray() override final;
        virtual void BindVertexArray(const sVertexArray* vertexArray) override final;
        virtual void BindDefaultVertexArray(const std::vector<sBuffer*>& buffersToBind) override final;
        virtual void UnbindVertexArray() override final;
        virtual void DestroyVertexArray(sVertexArray* vertexArray) override final;
        virtual void BindShader(const sShader* shader) override final;
        virtual void UnbindShader() override final;
        virtual sShader* CreateShader(eCategory renderPath, const std::string& vertexPath, const std::string& fragmentPath, const std::vector<sShader::sDefinePair>& definePairs = {}) override final;
        virtual sShader* CreateShader(const sShader* baseShader, const std::string& vertexFunc, const std::string& fragmentFunc, const std::vector<sShader::sDefinePair>& definePairs = {}) override final;
        virtual void DefineInShader(sShader* shader, const std::vector<sShader::sDefinePair>& definePairs) override final;
        virtual void DestroyShader(sShader* shader) override final;
        virtual void SetShaderUniform(const sShader* shader, const std::string& name, const glm::mat4& matrix) override final;
        virtual void SetShaderUniform(const sShader* shader, const std::string& name, types::usize count, const types::f32* values) override final;
        virtual sTexture* CreateTexture(types::usize width, types::usize height, types::usize depth, sTexture::eType type, sTexture::eFormat format, const void* data) override final;
        virtual sTexture* ResizeTexture(sTexture* texture, const glm::vec2& size) override final;
        virtual void BindTexture(const sShader* shader, const std::string& name, const sTexture* texture, types::s32 slot) override final;
        virtual void UnbindTexture(const sTexture* texture) override final;
        virtual void WriteTexture(const sTexture* texture, const glm::vec3& offset, const glm::vec2& size, const void* data) override final;
        virtual void WriteTextureToFile(const sTexture* texture, const std::string& filename) override final;
        virtual void GenerateTextureMips(const sTexture* texture) override final;
        virtual void DestroyTexture(sTexture* texture) override final;
        virtual sRenderTarget* CreateRenderTarget(const std::vector<sTexture*>& colorAttachments, sTexture* depthAttachment) override final;
        virtual void ResizeRenderTargetColors(sRenderTarget* renderTarget, const glm::vec2& size) override final;
        virtual void ResizeRenderTargetDepth(sRenderTarget* renderTarget, const glm::vec2& size) override final;
        virtual void UpdateRenderTargetBuffers(sRenderTarget* renderTarget) override final;
        virtual void BindRenderTarget(const sRenderTarget* renderTarget) override final;
        virtual void UnbindRenderTarget() override final;
        virtual void DestroyRenderTarget(sRenderTarget* renderTarget) override final;
        virtual sRenderPass* CreateRenderPass(const sRenderPass::sDescriptor& descriptor) override final;
        virtual void BindRenderPass(const sRenderPass* renderPass, sShader* customShader = nullptr) override final;
        virtual void UnbindRenderPass(const sRenderPass* renderPass) override final;
        virtual void DestroyRenderPass(sRenderPass* renderPass) override final;
        virtual void BindDefaultInputLayout() override final;
        virtual void BindDepthMode(const sDepthMode& blendMode) override final;
        virtual void BindBlendMode(const sBlendMode& blendMode) override final;
        virtual void Viewport(const glm::vec4& viewport) override final;
        virtual void ClearColor(const glm::vec4& color) override final;
        virtual void ClearDepth(types::f32 depth) override final;
        virtual void ClearFramebufferColor(types::usize bufferIndex, const glm::vec4& color) override final;
        virtual void ClearFramebufferDepth(types::f32 depth) override final;
        virtual void Draw(types::usize indexCount, types::usize vertexOffset, types::usize indexOffset, types::usize instanceCount) override final;
        virtual void DrawQuad() override final;
        virtual void DrawQuads(types::usize count) override final;
    };
}