// render_context_gl.cpp

#include <iostream>
#include <cstring>
#include <string>
#include <lodepng.h>
#include <GL/glew.h>
#include "render_context.hpp"
#include "filesystem_manager.hpp"
#include "types.hpp"
#include "application.hpp"
#include "render_manager.hpp"
#include "memory_pool.hpp"
#include "log.hpp"

using namespace types;

namespace realware
{
    void APIENTRY GLDebugCallback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam
    )
    {
        Print(message);
    }

    std::string CleanShaderSource(const std::string& src)
    {
        std::string out;
        out.reserve(src.size());
        for (u8 c : src)
        {
            if (c == '\t' || c == '\n' || c == '\r' || (c >= 32 && c <= 126))
                out.push_back(c);
        }

        return out;
    }

    cOpenGLRenderContext::cOpenGLRenderContext(cContext* context) : iRenderContext(context)
    {
        if (glewInit() != GLEW_OK)
        {
            Print("Error: can't initialize GLEW!");
            return;
        }

        glEnable(GL_DEPTH_TEST);
        //glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glDepthFunc(GL_LESS);
        //glCullFace(GL_BACK);
        glFrontFace(GL_CW);
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugCallback, nullptr);
    }

    cOpenGLRenderContext::~cOpenGLRenderContext()
    {
    }

    sBuffer* cOpenGLRenderContext::CreateBuffer(usize byteSize, sBuffer::eType type, const void* data)
    {
        sBuffer* pBuffer = (sBuffer*)GetApplication()->GetMemoryPool()->Allocate(sizeof(sBuffer));
        sBuffer* buffer = new (pBuffer) sBuffer();
        buffer->_byteSize = byteSize;
        buffer->_type = type;
        buffer->_slot = 0;

        glGenBuffers(1, (GLuint*)&buffer->_instance);

        if (buffer->_type == sBuffer::eType::VERTEX)
        {
            glBindBuffer(GL_ARRAY_BUFFER, (GLuint)buffer->_instance);
            glBufferData(GL_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else if (buffer->_type == sBuffer::eType::INDEX)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)buffer->_instance);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, byteSize, data, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        else if (buffer->_type == sBuffer::eType::UNIFORM)
        {
            glBindBuffer(GL_UNIFORM_BUFFER, (GLuint)buffer->_instance);
            glBufferData(GL_UNIFORM_BUFFER, byteSize, data, GL_STATIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
        else if (buffer->_type == sBuffer::eType::LARGE)
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, (GLuint)buffer->_instance);
            glBufferData(GL_SHADER_STORAGE_BUFFER, byteSize, data, GL_STATIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }

        return buffer;
    }

    void cOpenGLRenderContext::BindBuffer(const sBuffer* buffer)
    {
        if (buffer->_type == sBuffer::eType::VERTEX)
            glBindBuffer(GL_ARRAY_BUFFER, (GLuint)buffer->_instance);
        else if (buffer->_type == sBuffer::eType::INDEX)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)buffer->_instance);
        else if (buffer->_type == sBuffer::eType::UNIFORM)
            glBindBufferBase(GL_UNIFORM_BUFFER, buffer->_slot, (GLuint)buffer->_instance);
        else if (buffer->_type == sBuffer::eType::LARGE)
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer->_slot, buffer->_instance);
    }
		
	void cOpenGLRenderContext::BindBufferNotVAO(const sBuffer* buffer)
    {
        if (buffer->_type == sBuffer::eType::UNIFORM)
            glBindBufferBase(GL_UNIFORM_BUFFER, buffer->_slot, (GLuint)buffer->_instance);
        else if (buffer->_type == sBuffer::eType::LARGE)
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer->_slot, buffer->_instance);
    }

    void cOpenGLRenderContext::UnbindBuffer(const sBuffer* buffer)
    {
        if (buffer->_type == sBuffer::eType::VERTEX)
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        else if (buffer->_type == sBuffer::eType::INDEX)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        else if (buffer->_type == sBuffer::eType::UNIFORM)
            glBindBufferBase(GL_UNIFORM_BUFFER, buffer->_slot, 0);
        else if (buffer->_type == sBuffer::eType::LARGE)
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer->_slot, 0);
    }

    void cOpenGLRenderContext::WriteBuffer(const sBuffer* buffer, usize offset, usize byteSize, const void* data)
    {
        if (buffer->_type == sBuffer::eType::VERTEX)
        {
            glBindBuffer(GL_ARRAY_BUFFER, buffer->_instance);
            glBufferSubData(GL_ARRAY_BUFFER, offset, byteSize, data);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else if (buffer->_type == sBuffer::eType::INDEX)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->_instance);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, byteSize, data);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        else if (buffer->_type == sBuffer::eType::UNIFORM)
        {
            glBindBuffer(GL_UNIFORM_BUFFER, buffer->_instance);
            glBufferSubData(GL_UNIFORM_BUFFER, offset, byteSize, data);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
        else if (buffer->_type == sBuffer::eType::LARGE)
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->_instance);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, byteSize, data);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }
    }

    void cOpenGLRenderContext::DestroyBuffer(sBuffer* buffer)
    {
        if (buffer->_type == sBuffer::eType::VERTEX)
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        else if (buffer->_type == sBuffer::eType::INDEX)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        else if (buffer->_type == sBuffer::eType::UNIFORM)
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        else if (buffer->_type == sBuffer::eType::LARGE)
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glDeleteBuffers(1, (GLuint*)&buffer->_instance);

        if (buffer != nullptr)
        {
            buffer->~sBuffer();
            GetApplication()->GetMemoryPool()->Free(buffer);
        }
    }

    sVertexArray* cOpenGLRenderContext::CreateVertexArray()
    {
        sVertexArray* pVertexArray = (sVertexArray*)GetApplication()->GetMemoryPool()->Allocate(sizeof(sVertexArray));
        sVertexArray* vertexArray = new (pVertexArray) sVertexArray();

        glGenVertexArrays(1, (GLuint*)&vertexArray->_instance);

        return vertexArray;
    }

    void cOpenGLRenderContext::BindVertexArray(const sVertexArray* vertexArray)
    {
        glBindVertexArray((GLuint)vertexArray->_instance);
    }

    void cOpenGLRenderContext::BindDefaultVertexArray(const std::vector<sBuffer*>& buffersToBind)
    {
        static sVertexArray* vertexArray = nullptr;

        if (vertexArray == nullptr)
        {
            vertexArray = CreateVertexArray();

            BindVertexArray(vertexArray);
            for (auto buffer : buffersToBind)
                BindBuffer(buffer);
            BindDefaultInputLayout();
            UnbindVertexArray();
        }

        BindVertexArray(vertexArray);
    }

    void cOpenGLRenderContext::UnbindVertexArray()
    {
        glBindVertexArray(0);
    }

    void cOpenGLRenderContext::DestroyVertexArray(sVertexArray* vertexArray)
    {
        glDeleteVertexArrays(1, (GLuint*)&vertexArray->_instance);

        if (vertexArray != nullptr)
        {
            vertexArray->~sVertexArray();
            GetApplication()->GetMemoryPool()->Free(vertexArray);
        }
    }

    void cOpenGLRenderContext::BindShader(const sShader* shader)
    {
        const GLuint shaderID = (GLuint)shader->_instance;
        glUseProgram(shaderID);
    }

    void cOpenGLRenderContext::UnbindShader()
    {
        glUseProgram(0);
    }

    sShader* cOpenGLRenderContext::CreateShader(eCategory renderPath, const std::string& vertexPath, const std::string& fragmentPath, const std::vector<sShader::sDefinePair>& definePairs)
    {
        cApplication* app = GetApplication();
        sShader* pShader = (sShader*)app->GetMemoryPool()->Allocate(sizeof(sShader));
        sShader* shader = new (pShader) sShader();

        std::string header = "";
        switch (renderPath)
        {
            case eCategory::RENDER_PATH_NONE:
                Print("Error: invalid 'RENDER_PATH_NONE' for shaders '" + vertexPath + "' and '" + fragmentPath + "'!");
                return nullptr;

            case eCategory::RENDER_PATH_OPAQUE:
                header = "RENDER_PATH_OPAQUE";
                break;

            case eCategory::RENDER_PATH_TRANSPARENT:
                header = "RENDER_PATH_TRANSPARENT";
                break;

            case eCategory::RENDER_PATH_TEXT:
                header = "RENDER_PATH_TEXT";
                break;

            case eCategory::RENDER_PATH_TRANSPARENT_COMPOSITE:
                header = "RENDER_PATH_TRANSPARENT_COMPOSITE";
                break;

            case eCategory::RENDER_PATH_QUAD:
                header = "RENDER_PATH_QUAD";
                break;
        }

        const std::string appendStr = "#version 430\n\n#define " + header + "\n\n";

        sFile* vertexShaderFile = app->GetFileSystemManager()->CreateDataFile(vertexPath, K_TRUE);
        shader->_vertex = CleanShaderSource(std::string((const char*)vertexShaderFile->_data));
        sFile* fragmentShaderFile = app->GetFileSystemManager()->CreateDataFile(fragmentPath, K_TRUE);
        shader->_fragment = CleanShaderSource(std::string((const char*)fragmentShaderFile->_data));
            
        DefineInShader(shader, definePairs);

        shader->_vertex = appendStr + shader->_vertex;
        shader->_fragment = appendStr + shader->_fragment;

        const char* vertex = shader->_vertex.c_str();
        const char* fragment = shader->_fragment.c_str();

        const GLint vertexByteSize = strlen(vertex);
        const GLint fragmentByteSize = strlen(fragment);
        shader->_instance = glCreateProgram();
        const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(vertexShader, 1, &vertex, &vertexByteSize);
        glShaderSource(fragmentShader, 1, &fragment, &fragmentByteSize);
        glCompileShader(vertexShader);
        glCompileShader(fragmentShader);
        glAttachShader(shader->_instance, vertexShader);
        glAttachShader(shader->_instance, fragmentShader);
        glLinkProgram(shader->_instance);
			
        GLint success;
        glGetProgramiv((GLuint)shader->_instance, GL_LINK_STATUS, &success);
        if (!success)
            Print("Error: can't link shader!");
        if (!glIsProgram((GLuint)shader->_instance))
            Print("Error: invalid shader!");

        GLint logBufferByteSize = 0;
        GLchar logBuffer[1024] = {};
        glGetShaderInfoLog(vertexShader, 1024, &logBufferByteSize, &logBuffer[0]);
        if (logBufferByteSize > 0)
        {
            Print("Error: vertex shader, header: " + header + ", path: " + vertexPath + "!");
            Print(logBuffer);
        }
        logBufferByteSize = 0;
        glGetShaderInfoLog(fragmentShader, 1024, &logBufferByteSize, &logBuffer[0]);
        if (logBufferByteSize > 0)
        {
            Print("Error: fragment shader, header: " + header + ", path: " + fragmentPath + "!");
            Print(logBuffer);
        }
			
		glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        app->GetFileSystemManager()->DestroyDataFile(vertexShaderFile);
        app->GetFileSystemManager()->DestroyDataFile(fragmentShaderFile);

        return shader;
    }

    sShader* cOpenGLRenderContext::CreateShader(const sShader* baseShader, const std::string& vertexFunc, const std::string& fragmentFunc, const std::vector<sShader::sDefinePair>& definePairs)
    {
        sShader* pShader = (sShader*)GetApplication()->GetMemoryPool()->Allocate(sizeof(sShader));
        sShader* shader = new (pShader) sShader();

        const std::string vertexFuncDefinition = "void Vertex_Func(in vec3 _positionLocal, in vec2 _texcoord, in vec3 _normal, in int _instanceID, in Instance _instance, in Material material, in float _use2D, out vec4 _glPosition){}";
        const std::string vertexFuncPassthroughCall = "Vertex_Passthrough(InPositionLocal, instance, instance.Use2D, gl_Position);";
        const std::string fragmentFuncDefinition = "void Fragment_Func(in vec2 _texcoord, in vec4 _textureColor, in vec4 _materialDiffuseColor, out vec4 _fragColor){}";
        const std::string fragmentFuncPassthroughCall = "Fragment_Passthrough(textureColor, DiffuseColor, fragColor);";

        shader->_vertex = baseShader->_vertex;
        shader->_fragment = baseShader->_fragment;

        const usize vertexFuncDefinitionPos = shader->_vertex.find(vertexFuncDefinition);
        if (vertexFuncDefinitionPos != std::string::npos)
            shader->_vertex.replace(vertexFuncDefinitionPos, vertexFuncDefinition.length(), vertexFunc);
        const usize vertexFuncPasstroughCallPos = shader->_vertex.find(vertexFuncPassthroughCall);
        if (vertexFuncPasstroughCallPos != std::string::npos)
            shader->_vertex.replace(vertexFuncPasstroughCallPos, vertexFuncPassthroughCall.length(), "");

        const usize fragmentFuncDefinitionPos = shader->_fragment.find(fragmentFuncDefinition);
        if (fragmentFuncDefinitionPos != std::string::npos)
            shader->_fragment.replace(fragmentFuncDefinitionPos, fragmentFuncDefinition.length(), fragmentFunc);
        const usize fragmentFuncPassthroughPos = shader->_fragment.find(fragmentFuncPassthroughCall);
        if (fragmentFuncPassthroughPos != std::string::npos)
            shader->_fragment.replace(fragmentFuncPassthroughPos, fragmentFuncPassthroughCall.length(), "");

        shader->_vertex = CleanShaderSource(shader->_vertex);
        shader->_fragment = CleanShaderSource(shader->_fragment);

        DefineInShader(shader, definePairs);

        const usize vertexVersionPos = shader->_vertex.find("#version 430");
        if (vertexVersionPos != std::string::npos)
            shader->_vertex.replace(vertexVersionPos, std::string("#version 430").length(), "");
        const usize fragmentVersionPos = shader->_fragment.find("#version 430");
        if (fragmentVersionPos != std::string::npos)
            shader->_fragment.replace(fragmentVersionPos, std::string("#version 430").length(), "");

        shader->_vertex = "#version 430\n\n" + shader->_vertex;
        shader->_fragment = "#version 430\n\n" + shader->_fragment;

        const char* vertex = shader->_vertex.c_str();
        const char* fragment = shader->_fragment.c_str();
        const GLint vertexByteSize = strlen(vertex);
        const GLint fragmentByteSize = strlen(fragment);
        shader->_instance = glCreateProgram();
        const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(vertexShader, 1, &vertex, &vertexByteSize);
        glShaderSource(fragmentShader, 1, &fragment, &fragmentByteSize);
        glCompileShader(vertexShader);
        glCompileShader(fragmentShader);
        glAttachShader(shader->_instance, vertexShader);
        glAttachShader(shader->_instance, fragmentShader);
        glLinkProgram(shader->_instance);

        GLint success;
        glGetProgramiv((GLuint)shader->_instance, GL_LINK_STATUS, &success);
        if (!success)
            Print("Error: can't link shader!");
        if (!glIsProgram((GLuint)shader->_instance))
            Print("Error: invalid shader!");

        GLint logBufferByteSize = 0;
        GLchar logBuffer[1024] = {};
        glGetShaderInfoLog(vertexShader, 1024, &logBufferByteSize, &logBuffer[0]);
        if (logBufferByteSize > 0)
        {
            Print("Error: vertex shader!");
            Print(logBuffer);
        }
        logBufferByteSize = 0;
        glGetShaderInfoLog(fragmentShader, 1024, &logBufferByteSize, &logBuffer[0]);
        if (logBufferByteSize > 0)
        {
            Print("Error: fragment shader!");
            Print(logBuffer);
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return shader;
    }

    void cOpenGLRenderContext::DefineInShader(sShader* shader, const std::vector<sShader::sDefinePair>& definePairs)
    {
        if (!definePairs.empty())
        {
            std::string defineStr = "";
            for (const auto& define : definePairs)
                defineStr += "#define " + define.Name + " " + std::to_string(define.Index) + "\n";

            shader->_vertex = defineStr + shader->_vertex;
            shader->_fragment = defineStr + shader->_fragment;
        }
    }

    void cOpenGLRenderContext::DestroyShader(sShader* shader)
    {
        glDeleteProgram(shader->_instance);

        if (shader != nullptr)
        {
            shader->~sShader();
            GetApplication()->GetMemoryPool()->Free(shader);
        }
    }

    void cOpenGLRenderContext::SetShaderUniform(const sShader* shader, const std::string& name, const glm::mat4& matrix)
    {
        glUniformMatrix4fv(glGetUniformLocation(shader->_instance, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
    }

    void cOpenGLRenderContext::SetShaderUniform(const sShader* shader, const std::string& name, usize count, const f32* values)
    {
        glUniform4fv(glGetUniformLocation(shader->_instance, name.c_str()), count, &values[0]);
    }

    sTexture* cOpenGLRenderContext::CreateTexture(usize width, usize height, usize depth, sTexture::eType type, sTexture::eFormat format, const void* data)
    {
        sTexture* pTexture = (sTexture*)GetApplication()->GetMemoryPool()->Allocate(sizeof(sTexture));
        sTexture* texture = new (pTexture) sTexture();
            
        texture->_width = width;
        texture->_height = height;
        texture->_depth = depth;
        texture->_type = type;
        texture->_format = format;

        glGenTextures(1, (GLuint*)&texture->_instance);

        GLenum formatGL = GL_RGBA8;
        GLenum channelsGL = GL_RGBA;
        GLenum formatComponentGL = GL_UNSIGNED_BYTE;
        if (texture->_format == sTexture::eFormat::R8)
        {
            formatGL = GL_R8;
            channelsGL = GL_RED;
            formatComponentGL = GL_UNSIGNED_BYTE;
        }
        else if (texture->_format == sTexture::eFormat::R8F)
        {
            formatGL = GL_R8;
            channelsGL = GL_RED;
            formatComponentGL = GL_FLOAT;
        }
        else if (texture->_format == sTexture::eFormat::RGBA8 || texture->_format == sTexture::eFormat::RGBA8_MIPS)
        {
            formatGL = GL_RGBA8;
            channelsGL = GL_RGBA;
            formatComponentGL = GL_UNSIGNED_BYTE;
        }
        else if (texture->_format == sTexture::eFormat::RGB16F)
        {
            formatGL = GL_RGB16F;
            channelsGL = GL_RGB;
            formatComponentGL = GL_HALF_FLOAT;
        }
        else if (texture->_format == sTexture::eFormat::RGBA16F)
        {
            formatGL = GL_RGBA16F;
            channelsGL = GL_RGBA;
            formatComponentGL = GL_HALF_FLOAT;
        }
        else if (texture->_format == sTexture::eFormat::DEPTH_STENCIL)
        {
            formatGL = GL_DEPTH24_STENCIL8;
            channelsGL = GL_DEPTH_STENCIL;
            formatComponentGL = GL_UNSIGNED_INT_24_8;
        }

        if (texture->_type == sTexture::eType::TEXTURE_2D)
        {
            glBindTexture(GL_TEXTURE_2D, texture->_instance);

            glTexImage2D(GL_TEXTURE_2D, 0, formatGL, texture->_width, texture->_height, 0, channelsGL, formatComponentGL, data);
            if (texture->_format != sTexture::eFormat::DEPTH_STENCIL)
            {
                if (texture->_format == sTexture::eFormat::RGBA8_MIPS)
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else if (texture->_type == sTexture::eType::TEXTURE_2D_ARRAY)
        {
            glBindTexture(GL_TEXTURE_2D_ARRAY, texture->_instance);

            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, formatGL, texture->_width, texture->_height, texture->_depth, 0, channelsGL, formatComponentGL, data);
                
            if (texture->_format == sTexture::eFormat::RGBA8_MIPS)
            {
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            else
            {
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        }

        return texture;
    }

    sTexture* cOpenGLRenderContext::ResizeTexture(sTexture* texture, const glm::vec2& size)
    {
        sTexture textureCopy = *texture;
        DestroyTexture(texture);

        sTexture* newTexture = CreateTexture(size.x, size.y, textureCopy._depth, textureCopy._type, textureCopy._format, nullptr);
            
        return newTexture;
    }

    void cOpenGLRenderContext::BindTexture(const sShader* shader, const std::string& name, const sTexture* texture, s32 slot)
    {
        if (slot == -1)
            slot = texture->_slot;

        if (texture->_type == sTexture::eType::TEXTURE_2D)
        {
            glUniform1i(glGetUniformLocation(shader->_instance, name.c_str()), slot);
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, texture->_instance);
            glActiveTexture(GL_TEXTURE0);
        }
        else if (texture->_type == sTexture::eType::TEXTURE_2D_ARRAY)
        {
            glUniform1i(glGetUniformLocation(shader->_instance, name.c_str()), slot);
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D_ARRAY, texture->_instance);
            glActiveTexture(GL_TEXTURE0);
        }
    }

    void cOpenGLRenderContext::UnbindTexture(const sTexture* texture)
    {
        if (texture->_type == sTexture::eType::TEXTURE_2D_ARRAY)
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    void cOpenGLRenderContext::WriteTexture(const sTexture* texture, const glm::vec3& offset, const glm::vec2& size, const void* data)
    {
        GLenum formatGL = GL_RGBA8;
        GLenum channelsGL = GL_RGBA;
        GLenum formatComponentGL = GL_UNSIGNED_BYTE;

        if (texture->_format == sTexture::eFormat::R8)
        {
            formatGL = GL_R8;
            channelsGL = GL_RED;
            formatComponentGL = GL_UNSIGNED_BYTE;
        }
        else if (texture->_format == sTexture::eFormat::R8F)
        {
            formatGL = GL_R8;
            channelsGL = GL_RED;
            formatComponentGL = GL_FLOAT;
        }
        else if (texture->_format == sTexture::eFormat::RGBA8 || texture->_format == sTexture::eFormat::RGBA8_MIPS)
        {
            formatGL = GL_RGBA8;
            channelsGL = GL_RGBA;
            formatComponentGL = GL_UNSIGNED_BYTE;
        }
        else if (texture->_format == sTexture::eFormat::RGB16F)
        {
            formatGL = GL_RGB16F;
            channelsGL = GL_RGB;
            formatComponentGL = GL_HALF_FLOAT;
        }
        else if (texture->_format == sTexture::eFormat::RGBA16F)
        {
            formatGL = GL_RGBA16F;
            channelsGL = GL_RGBA;
            formatComponentGL = GL_HALF_FLOAT;
        }
        else if (texture->_format == sTexture::eFormat::DEPTH_STENCIL)
        {
            formatGL = GL_DEPTH24_STENCIL8;
            channelsGL = GL_DEPTH_STENCIL;
            formatComponentGL = GL_UNSIGNED_INT_24_8;
        }

        if (texture->_type == sTexture::eType::TEXTURE_2D)
        {
            glBindTexture(GL_TEXTURE_2D, texture->_instance);
            glTexSubImage2D(GL_TEXTURE_2D, 0, offset.x, offset.y, size.x, size.y, channelsGL, formatComponentGL, data);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else if (texture->_type == sTexture::eType::TEXTURE_2D_ARRAY)
        {
            if (offset.x + size.x <= texture->_width && offset.y + size.y <= texture->_height && offset.z < texture->_depth)
            {
                glBindTexture(GL_TEXTURE_2D_ARRAY, texture->_instance);
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, offset.x, offset.y, offset.z, size.x, size.y, 1, channelsGL, formatComponentGL, data);
                glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            }
        }
    }

    void cOpenGLRenderContext::WriteTextureToFile(const sTexture* const texture, const std::string& filename)
    {
        if (texture->_format != sTexture::eFormat::RGBA8)
            return;

        GLenum channelsGL = GL_RGBA;
        GLenum formatComponentGL = GL_UNSIGNED_BYTE;
        usize formatByteCount = 4;

        if (texture->_format == sTexture::eFormat::RGBA8)
        {
            channelsGL = GL_RGBA;
            formatComponentGL = GL_UNSIGNED_BYTE;
            formatByteCount = 4;
        }

        if (texture->_type == sTexture::eType::TEXTURE_2D)
        {
            cMemoryPool* memoryPool = GetApplication()->GetMemoryPool();

            unsigned char* pixels = (unsigned char*)memoryPool->Allocate(texture->_width * texture->_height * 4);

            glBindTexture(GL_TEXTURE_2D, texture->_instance);
            glGetTexImage(GL_TEXTURE_2D, 0, channelsGL, formatComponentGL, pixels);
            glBindTexture(GL_TEXTURE_2D, 0);

            lodepng_encode32_file(filename.c_str(), (const unsigned char*)pixels, texture->_width, texture->_height);

            memoryPool->Free(pixels);
        }
    }

    void cOpenGLRenderContext::GenerateTextureMips(const sTexture* texture)
    {
        if (texture->_type == sTexture::eType::TEXTURE_2D)
        {
            glBindTexture(GL_TEXTURE_2D, texture->_instance);
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else if (texture->_type == sTexture::eType::TEXTURE_2D_ARRAY)
        {
            glBindTexture(GL_TEXTURE_2D_ARRAY, texture->_instance);
            glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        }
    }

    void cOpenGLRenderContext::DestroyTexture(sTexture* texture)
    {
        if (texture->_type == sTexture::eType::TEXTURE_2D)
            glBindTexture(GL_TEXTURE_2D, 0);
        else if (texture->_type == sTexture::eType::TEXTURE_2D_ARRAY)
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

        glDeleteTextures(1, (GLuint*)&texture->_instance);

        if (texture != nullptr)
        {
            texture->~sTexture();
            GetApplication()->GetMemoryPool()->Free(texture);
        }
    }

    sRenderTarget* cOpenGLRenderContext::CreateRenderTarget(const std::vector<sTexture*>& colorAttachments, sTexture* depthAttachment)
    {
        sRenderTarget* pRenderTarget = (sRenderTarget*)GetApplication()->GetMemoryPool()->Allocate(sizeof(sRenderTarget));
        sRenderTarget* renderTarget = new (pRenderTarget) sRenderTarget();

        renderTarget->_colorAttachments = colorAttachments;
        renderTarget->_depthAttachment = depthAttachment;

        GLenum buffs[16] = {};
        glGenFramebuffers(1, (GLuint*)&renderTarget->_instance);
        glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->_instance);
        for (usize i = 0; i < renderTarget->_colorAttachments.size(); i++)
        {
            buffs[i] = GL_COLOR_ATTACHMENT0 + i;
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, renderTarget->_colorAttachments[i]->_instance, 0);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderTarget->_depthAttachment->_instance, 0);
        glDrawBuffers(renderTarget->_colorAttachments.size(), &buffs[0]);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
		if (status != GL_FRAMEBUFFER_COMPLETE)
            Print("Error: incomplete framebuffer!");

        return renderTarget;
    }

    void cOpenGLRenderContext::ResizeRenderTargetColors(sRenderTarget* renderTarget, const glm::vec2& size)
    {
        std::vector<sTexture*> newColorAttachments;
        for (auto attachment : renderTarget->_colorAttachments)
        {
            sTexture attachmentCopy = *attachment;
            DestroyTexture(attachment);
            newColorAttachments.emplace_back(CreateTexture(size.x, size.y, attachmentCopy._depth, attachmentCopy._type, attachmentCopy._format, nullptr));
        }
        renderTarget->_colorAttachments.clear();
        renderTarget->_colorAttachments = newColorAttachments;

        GLenum buffs[16] = {};
        glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->_instance);
        for (usize i = 0; i < renderTarget->_colorAttachments.size(); i++)
        {
            buffs[i] = GL_COLOR_ATTACHMENT0 + i;
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, renderTarget->_colorAttachments[i]->_instance, 0);
        }
        glDrawBuffers(renderTarget->_colorAttachments.size(), &buffs[0]);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void cOpenGLRenderContext::ResizeRenderTargetDepth(sRenderTarget* renderTarget, const glm::vec2& size)
    {
        sTexture attachmentCopy = *renderTarget->_depthAttachment;
        DestroyTexture(renderTarget->_depthAttachment);

        sTexture* newDepthAttachment = CreateTexture(size.x, size.y, attachmentCopy._depth, attachmentCopy._type, attachmentCopy._format, nullptr);
        renderTarget->_depthAttachment = newDepthAttachment;

        GLenum buffs[16] = {};
        glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->_instance);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderTarget->_depthAttachment->_instance, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void cOpenGLRenderContext::UpdateRenderTargetBuffers(sRenderTarget* renderTarget)
    {
        GLenum buffs[16] = {};
        glGenFramebuffers(1, (GLuint*)&renderTarget->_instance);
        glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->_instance);
        for (usize i = 0; i < renderTarget->_colorAttachments.size(); i++)
        {
            buffs[i] = GL_COLOR_ATTACHMENT0 + i;
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, renderTarget->_colorAttachments[i]->_instance, 0);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderTarget->_depthAttachment->_instance, 0);
        glDrawBuffers(renderTarget->_colorAttachments.size(), &buffs[0]);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void cOpenGLRenderContext::BindRenderTarget(const sRenderTarget* renderTarget)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->_instance);
    }

    void cOpenGLRenderContext::UnbindRenderTarget()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void cOpenGLRenderContext::DestroyRenderTarget(sRenderTarget* renderTarget)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, (GLuint*)&renderTarget->_instance);

        if (renderTarget != nullptr)
        {
            renderTarget->~sRenderTarget();
            GetApplication()->GetMemoryPool()->Free(renderTarget);
        }
    }

    sRenderPass* cOpenGLRenderContext::CreateRenderPass(const sRenderPass::sDescriptor& descriptor)
    {
        sRenderPass* pRenderPass = (sRenderPass*)GetApplication()->GetMemoryPool()->Allocate(sizeof(sRenderPass));
        sRenderPass* renderPass = new (pRenderPass) sRenderPass();
        memset(renderPass, 0, sizeof(sRenderPass));
        renderPass->_desc = descriptor;

        std::vector<sShader::sDefinePair> definePairs = {};

        if (renderPass->_desc._inputTextureAtlasTextures.size() != renderPass->_desc._inputTextureAtlasTextureNames.size())
        {
            Print("Error: mismatch of render pass input texture atlas texture array and input texture atlas texture name array!");
            return nullptr;
        }
        for (usize i = 0; i < renderPass->_desc._inputTextureAtlasTextures.size(); i++)
        {
            const usize textureAtlasTextureIndex = i;
            const std::string& textureAtlasTextureName = renderPass->_desc._inputTextureAtlasTextureNames[i];
            definePairs.push_back({ textureAtlasTextureName, textureAtlasTextureIndex });
        }

        if (renderPass->_desc._shaderBase == nullptr)
        {
            renderPass->_desc._shader = CreateShader(
                renderPass->_desc._shaderRenderPath,
                renderPass->_desc._shaderVertexPath,
                renderPass->_desc._shaderFragmentPath,
                definePairs
            );
        }
        else
        {
            renderPass->_desc._shader = CreateShader(
                renderPass->_desc._shaderBase,
                renderPass->_desc._shaderVertexFunc,
                renderPass->_desc._shaderFragmentFunc,
                definePairs
            );
        }

        renderPass->_desc._vertexArray = CreateVertexArray();
        BindVertexArray(renderPass->_desc._vertexArray);
        if (renderPass->_desc._inputVertexFormat == eCategory::VERTEX_BUFFER_FORMAT_NONE)
        {
            for (auto buffer : renderPass->_desc._inputBuffers)
                BindBuffer(buffer);
        }
        else if (renderPass->_desc._inputVertexFormat == eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3)
        {
            for (auto buffer : renderPass->_desc._inputBuffers)
                BindBuffer(buffer);

            BindDefaultInputLayout();
        }

        UnbindVertexArray();

        return renderPass;
    }

    void cOpenGLRenderContext::BindRenderPass(const sRenderPass* renderPass, sShader* customShader)
    {
        sShader* shader = nullptr;
        if (customShader == nullptr)
            shader = renderPass->_desc._shader;
        else
            shader = customShader;

        BindShader(shader);
        BindVertexArray(renderPass->_desc._vertexArray);
        if (renderPass->_desc._renderTarget != nullptr)
            BindRenderTarget(renderPass->_desc._renderTarget);
        else
            UnbindRenderTarget();
        Viewport(renderPass->_desc._viewport);
        for (auto buffer : renderPass->_desc._inputBuffers)
            BindBufferNotVAO(buffer);
        BindDepthMode(renderPass->_desc._depthMode);
        BindBlendMode(renderPass->_desc._blendMode);
        for (usize i = 0; i < renderPass->_desc._inputTextures.size(); i++)
            BindTexture(shader, renderPass->_desc._inputTextureNames[i].c_str(), renderPass->_desc._inputTextures[i], i);
    }

    void cOpenGLRenderContext::UnbindRenderPass(const sRenderPass* renderPass)
    {
        UnbindVertexArray();
        if (renderPass->_desc._renderTarget != nullptr)
            UnbindRenderTarget();
        for (auto buffer : renderPass->_desc._inputBuffers)
            UnbindBuffer(buffer);
        for (auto texture : renderPass->_desc._inputTextures)
            UnbindTexture(texture);
    }

    void cOpenGLRenderContext::DestroyRenderPass(sRenderPass* renderPass)
    {
        glBindVertexArray(0);
        DestroyVertexArray(renderPass->_desc._vertexArray);

        if (renderPass != nullptr)
        {
            renderPass->~sRenderPass();
            GetApplication()->GetMemoryPool()->Free(renderPass);
        }
    }

    void cOpenGLRenderContext::BindDefaultInputLayout()
    {
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, (void*)12);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 32, (void*)20);
    }

    void cOpenGLRenderContext::BindBlendMode(const sBlendMode& blendMode)
    {
        for (usize i = 0; i < blendMode._factorCount; i++)
        {
            GLuint srcFactor = GL_ZERO;
            GLuint dstFactor = GL_ZERO;

            switch (blendMode._srcFactors[i])
            {
                case sBlendMode::eFactor::ONE: srcFactor = GL_ONE; break;
                case sBlendMode::eFactor::SRC_COLOR: srcFactor = GL_SRC_COLOR; break;
                case sBlendMode::eFactor::INV_SRC_COLOR: srcFactor = GL_ONE_MINUS_SRC_COLOR; break;
                case sBlendMode::eFactor::SRC_ALPHA: srcFactor = GL_SRC_ALPHA; break;
                case sBlendMode::eFactor::INV_SRC_ALPHA: srcFactor = GL_ONE_MINUS_SRC_ALPHA; break;
            }

            switch (blendMode._dstFactors[i])
            {
                case sBlendMode::eFactor::ONE: dstFactor = GL_ONE; break;
                case sBlendMode::eFactor::SRC_COLOR: dstFactor = GL_SRC_COLOR; break;
                case sBlendMode::eFactor::INV_SRC_COLOR: dstFactor = GL_ONE_MINUS_SRC_COLOR; break;
                case sBlendMode::eFactor::SRC_ALPHA: dstFactor = GL_SRC_ALPHA; break;
                case sBlendMode::eFactor::INV_SRC_ALPHA: dstFactor = GL_ONE_MINUS_SRC_ALPHA; break;
            }

            glBlendFunci(i, srcFactor, dstFactor);
        }
    }

    void cOpenGLRenderContext::BindDepthMode(const sDepthMode& blendMode)
    {
        if (blendMode._useDepthTest == K_TRUE)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);

        if (blendMode._useDepthWrite == K_TRUE)
            glDepthMask(GL_TRUE);
        else
            glDepthMask(GL_FALSE);
    }

    void cOpenGLRenderContext::Viewport(const glm::vec4& viewport)
    {
        glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
    }

    void cOpenGLRenderContext::ClearColor(const glm::vec4& color)
    {
        glClearColor(color.x, color.y, color.z, color.w);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void cOpenGLRenderContext::ClearDepth(const f32 depth)
    {
        glClearDepth(depth);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void cOpenGLRenderContext::ClearFramebufferColor(usize bufferIndex, const glm::vec4& color)
    {
        glClearBufferfv(GL_COLOR, bufferIndex, &color.x);
    }

    void cOpenGLRenderContext::ClearFramebufferDepth(f32 depth)
    {
        glClearDepth(depth);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void cOpenGLRenderContext::Draw(usize indexCount, usize vertexOffset, usize indexOffset, usize instanceCount)
    {
        glDrawElementsInstancedBaseVertex(
            GL_TRIANGLES,
            indexCount,
            GL_UNSIGNED_INT,
            (const void*)indexOffset,
            instanceCount,
            vertexOffset
        );
    }

    void cOpenGLRenderContext::DrawQuad()
    {
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void cOpenGLRenderContext::DrawQuads(usize count)
    {
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
    }
}