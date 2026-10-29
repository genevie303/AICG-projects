#include "graphics/gl_program.h"

#include <format>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "graphics/gl_utility.h"

namespace gfx {

std::string_view getVarTypeName(GLProgram::VarType type) noexcept {
    switch (type) {
    case GLProgram::VarType::Unknown:
        return "Unknown";
    case GLProgram::VarType::Bool:
        return "Bool";
    case GLProgram::VarType::BVec2:
        return "BVec2";
    case GLProgram::VarType::BVec3:
        return "BVec3";
    case GLProgram::VarType::BVec4:
        return "BVec4";
    case GLProgram::VarType::Int:
        return "Int";
    case GLProgram::VarType::IVec2:
        return "IVec2";
    case GLProgram::VarType::IVec3:
        return "IVec3";
    case GLProgram::VarType::IVec4:
        return "IVec4";
    case GLProgram::VarType::UInt:
        return "UInt";
    case GLProgram::VarType::UVec2:
        return "UVec2";
    case GLProgram::VarType::UVec3:
        return "UVec3";
    case GLProgram::VarType::UVec4:
        return "UVec4";
    case GLProgram::VarType::Float:
        return "Float";
    case GLProgram::VarType::Vec2:
        return "Vec2";
    case GLProgram::VarType::Vec3:
        return "Vec3";
    case GLProgram::VarType::Vec4:
        return "Vec4";
    case GLProgram::VarType::Double:
        return "Double";
    case GLProgram::VarType::DVec2:
        return "DVec2";
    case GLProgram::VarType::DVec3:
        return "DVec3";
    case GLProgram::VarType::DVec4:
        return "DVec4";
    case GLProgram::VarType::Mat2x2:
        return "Mat2x2";
    case GLProgram::VarType::Mat2x3:
        return "Mat2x3";
    case GLProgram::VarType::Mat2x4:
        return "Mat2x4";
    case GLProgram::VarType::Mat3x2:
        return "Mat3x2";
    case GLProgram::VarType::Mat3x3:
        return "Mat3x3";
    case GLProgram::VarType::Mat3x4:
        return "Mat3x4";
    case GLProgram::VarType::Mat4x2:
        return "Mat4x2";
    case GLProgram::VarType::Mat4x3:
        return "Mat4x3";
    case GLProgram::VarType::Mat4x4:
        return "Mat4x4";
    case GLProgram::VarType::DMat2x2:
        return "DMat2x2";
    case GLProgram::VarType::DMat2x3:
        return "DMat2x3";
    case GLProgram::VarType::DMat2x4:
        return "DMat2x4";
    case GLProgram::VarType::DMat3x2:
        return "DMat3x2";
    case GLProgram::VarType::DMat3x3:
        return "DMat3x3";
    case GLProgram::VarType::DMat3x4:
        return "DMat3x4";
    case GLProgram::VarType::DMat4x2:
        return "DMat4x2";
    case GLProgram::VarType::DMat4x3:
        return "DMat4x3";
    case GLProgram::VarType::DMat4x4:
        return "DMat4x4";
    case GLProgram::VarType::Sampler1D:
        return "Sampler1D";
    case GLProgram::VarType::Sampler2D:
        return "Sampler2D";
    case GLProgram::VarType::Sampler3D:
        return "Sampler3D";
    case GLProgram::VarType::SamplerCubemap:
        return "SamplerCubemap";
    case GLProgram::VarType::Sampler1DShadow:
        return "Sampler1DShadow";
    case GLProgram::VarType::Sampler2DShadow:
        return "Sampler2DShadow";
    case GLProgram::VarType::Sampler1DArray:
        return "Sampler1DArray";
    case GLProgram::VarType::Sampler2DArray:
        return "Sampler2DArray";
    case GLProgram::VarType::SamplerCubeArray:
        return "SamplerCubeArray";
    case GLProgram::VarType::Sampler1DArrayShadow:
        return "Sampler1DArrayShadow";
    case GLProgram::VarType::Sampler2DArrayShadow:
        return "Sampler2DArrayShadow";
    case GLProgram::VarType::Sampler2DMultiSample:
        return "Sampler2DMultiSample";
    case GLProgram::VarType::Sampler2DMultiSampleArray:
        return "Sampler2DMultiSampleArray";
    case GLProgram::VarType::SamplerCubemapShadow:
        return "SamplerCubemapShadow";
    case GLProgram::VarType::SamplerCubemapArrayShadow:
        return "SamplerCubemapArrayShadow";
    case GLProgram::VarType::SamplerBuffer:
        return "SamplerBuffer";
    case GLProgram::VarType::Sampler2DRect:
        return "Sampler2DRect";
    case GLProgram::VarType::Sampler2DRectShadow:
        return "Sampler2DRectShadow";
    case GLProgram::VarType::ISampler1D:
        return "ISampler1D";
    case GLProgram::VarType::ISampler2D:
        return "ISampler2D";
    case GLProgram::VarType::ISampler3D:
        return "ISampler3D";
    case GLProgram::VarType::ISamplerCubemap:
        return "ISamplerCubemap";
    case GLProgram::VarType::ISampler1DArray:
        return "ISampler1DArray";
    case GLProgram::VarType::ISampler2DArray:
        return "ISampler2DArray";
    case GLProgram::VarType::ISamplerCubemapArray:
        return "ISamplerCubemapArray";
    case GLProgram::VarType::ISampler2DMultiSample:
        return "ISampler2DMultiSample";
    case GLProgram::VarType::ISampler2DMultiSampleArray:
        return "ISampler2DMultiSampleArray";
    case GLProgram::VarType::ISamplerBuffer:
        return "ISamplerBuffer";
    case GLProgram::VarType::ISampler2DRect:
        return "ISampler2DRect";
    case GLProgram::VarType::USampler1D:
        return "USampler1D";
    case GLProgram::VarType::USampler2D:
        return "USampler2D";
    case GLProgram::VarType::USampler3D:
        return "USampler3D";
    case GLProgram::VarType::USamplerCubemap:
        return "USamplerCubemap";
    case GLProgram::VarType::USampler1DArray:
        return "USampler1DArray";
    case GLProgram::VarType::USampler2DArray:
        return "USampler2DArray";
    case GLProgram::VarType::USamplerCubemapArray:
        return "USamplerCubemapArray";
    case GLProgram::VarType::USampler2DMultiSample:
        return "USampler2DMultiSample";
    case GLProgram::VarType::USampler2DMultiSampleArray:
        return "USampler2DMultiSampleArray";
    case GLProgram::VarType::USamplerBuffer:
        return "USamplerBuffer";
    case GLProgram::VarType::USampler2DRect:
        return "USampler2DRect";
    case GLProgram::VarType::Image1D:
        return "Image1D";
    case GLProgram::VarType::Image2D:
        return "Image2D";
    case GLProgram::VarType::Image3D:
        return "Image3D";
    case GLProgram::VarType::Image2DRect:
        return "Image2DRect";
    case GLProgram::VarType::ImageCubemap:
        return "ImageCubemap";
    case GLProgram::VarType::ImageBuffer:
        return "ImageBuffer";
    case GLProgram::VarType::Image1DArray:
        return "Image1DArray";
    case GLProgram::VarType::Image2DArray:
        return "Image2DArray";
    case GLProgram::VarType::ImageCubemapArray:
        return "ImageCubemapArray";
    case GLProgram::VarType::Image2DMultiSample:
        return "Image2DMultiSample";
    case GLProgram::VarType::Image2DMultiSampleArray:
        return "Image2DMultiSampleArray";
    case GLProgram::VarType::IImage1D:
        return "IImage1D";
    case GLProgram::VarType::IImage2D:
        return "IImage2D";
    case GLProgram::VarType::IImage3D:
        return "IImage3D";
    case GLProgram::VarType::IImage2DRect:
        return "IImage2DRect";
    case GLProgram::VarType::IImageCubemap:
        return "IImageCubemap";
    case GLProgram::VarType::IImageBuffer:
        return "IImageBuffer";
    case GLProgram::VarType::IImage1DArray:
        return "IImage1DArray";
    case GLProgram::VarType::IImage2DArray:
        return "IImage2DArray";
    case GLProgram::VarType::IImageCubemapArray:
        return "IImageCubemapArray";
    case GLProgram::VarType::IImage2DMultiSample:
        return "IImage2DMultiSample";
    case GLProgram::VarType::IImage2DMultiSampleArray:
        return "IImage2DMultiSampleArray";
    case GLProgram::VarType::UImage1D:
        return "UImage1D";
    case GLProgram::VarType::UImage2D:
        return "UImage2D";
    case GLProgram::VarType::UImage3D:
        return "UImage3D";
    case GLProgram::VarType::UImage2DRect:
        return "UImage2DRect";
    case GLProgram::VarType::UImageCubemap:
        return "UImageCubemap";
    case GLProgram::VarType::UImageBuffer:
        return "UImageBuffer";
    case GLProgram::VarType::UImage1DArray:
        return "UImage1DArray";
    case GLProgram::VarType::UImage2DArray:
        return "UImage2DArray";
    case GLProgram::VarType::UImageCubemapArray:
        return "UImageCubemapArray";
    case GLProgram::VarType::UImage2DMultiSample:
        return "UImage2DMultiSample";
    case GLProgram::VarType::UImage2DMultiSampleArray:
        return "UImage2DMultiSampleArray";
    case GLProgram::VarType::AtomicCounter:
        return "AtomicCounter";
    case GLProgram::VarType::AccelerationStructure:
        return "AccelerationStructure";
    case GLProgram::VarType::RayQuery:
        return "RayQuery";
    }

    return "Unknown";
}

static GLProgram::VarType getVarType(GLint type) {
    switch (type) {
    default:
        break;
    case GL_BOOL:
        return GLProgram::VarType::Bool;
    case GL_BOOL_VEC2:
        return GLProgram::VarType::BVec2;
    case GL_BOOL_VEC3:
        return GLProgram::VarType::BVec3;
    case GL_BOOL_VEC4:
        return GLProgram::VarType::BVec4;
    case GL_INT:
        return GLProgram::VarType::Int;
    case GL_INT_VEC2:
        return GLProgram::VarType::IVec2;
    case GL_INT_VEC3:
        return GLProgram::VarType::IVec3;
    case GL_INT_VEC4:
        return GLProgram::VarType::IVec4;
    case GL_UNSIGNED_INT:
        return GLProgram::VarType::UInt;
    case GL_UNSIGNED_INT_VEC2:
        return GLProgram::VarType::UVec2;
    case GL_UNSIGNED_INT_VEC3:
        return GLProgram::VarType::UVec3;
    case GL_UNSIGNED_INT_VEC4:
        return GLProgram::VarType::UVec4;
    case GL_FLOAT:
        return GLProgram::VarType::Float;
    case GL_FLOAT_VEC2:
        return GLProgram::VarType::Vec2;
    case GL_FLOAT_VEC3:
        return GLProgram::VarType::Vec3;
    case GL_FLOAT_VEC4:
        return GLProgram::VarType::Vec4;
    case GL_DOUBLE:
        return GLProgram::VarType::Double;
    case GL_DOUBLE_VEC2:
        return GLProgram::VarType::DVec2;
    case GL_DOUBLE_VEC3:
        return GLProgram::VarType::DVec3;
    case GL_DOUBLE_VEC4:
        return GLProgram::VarType::DVec4;
    case GL_FLOAT_MAT2:
        return GLProgram::VarType::Mat2x2;
    case GL_FLOAT_MAT2x3:
        return GLProgram::VarType::Mat2x3;
    case GL_FLOAT_MAT2x4:
        return GLProgram::VarType::Mat2x4;
    case GL_FLOAT_MAT3x2:
        return GLProgram::VarType::Mat3x2;
    case GL_FLOAT_MAT3:
        return GLProgram::VarType::Mat3x3;
    case GL_FLOAT_MAT3x4:
        return GLProgram::VarType::Mat3x4;
    case GL_FLOAT_MAT4x2:
        return GLProgram::VarType::Mat4x2;
    case GL_FLOAT_MAT4x3:
        return GLProgram::VarType::Mat4x3;
    case GL_FLOAT_MAT4:
        return GLProgram::VarType::Mat4x4;
    case GL_DOUBLE_MAT2:
        return GLProgram::VarType::DMat2x2;
    case GL_DOUBLE_MAT2x3:
        return GLProgram::VarType::DMat2x3;
    case GL_DOUBLE_MAT2x4:
        return GLProgram::VarType::DMat2x4;
    case GL_DOUBLE_MAT3x2:
        return GLProgram::VarType::DMat3x2;
    case GL_DOUBLE_MAT3:
        return GLProgram::VarType::DMat3x3;
    case GL_DOUBLE_MAT3x4:
        return GLProgram::VarType::DMat3x4;
    case GL_DOUBLE_MAT4x2:
        return GLProgram::VarType::DMat4x2;
    case GL_DOUBLE_MAT4x3:
        return GLProgram::VarType::DMat4x3;
    case GL_DOUBLE_MAT4:
        return GLProgram::VarType::DMat4x4;
    case GL_SAMPLER_1D:
        return GLProgram::VarType::Sampler1D;
    case GL_SAMPLER_2D:
        return GLProgram::VarType::Sampler2D;
    case GL_SAMPLER_3D:
        return GLProgram::VarType::Sampler3D;
    case GL_SAMPLER_CUBE:
        return GLProgram::VarType::SamplerCubemap;
    case GL_SAMPLER_1D_ARRAY:
        return GLProgram::VarType::Sampler1DArray;
    case GL_SAMPLER_2D_ARRAY:
        return GLProgram::VarType::Sampler2DArray;
    case GL_SAMPLER_CUBE_MAP_ARRAY:
        return GLProgram::VarType::SamplerCubeArray;
    case GL_SAMPLER_1D_ARRAY_SHADOW:
        return GLProgram::VarType::Sampler1DArrayShadow;
    case GL_SAMPLER_2D_ARRAY_SHADOW:
        return GLProgram::VarType::Sampler2DArrayShadow;
    case GL_SAMPLER_2D_MULTISAMPLE:
        return GLProgram::VarType::Sampler2DMultiSample;
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        return GLProgram::VarType::Sampler2DMultiSampleArray;
    case GL_SAMPLER_CUBE_SHADOW:
        return GLProgram::VarType::SamplerCubemapShadow;
    case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
        return GLProgram::VarType::SamplerCubemapArrayShadow;
    case GL_SAMPLER_BUFFER:
        return GLProgram::VarType::SamplerBuffer;
    case GL_SAMPLER_2D_RECT:
        return GLProgram::VarType::Sampler2DRect;
    case GL_SAMPLER_2D_RECT_SHADOW:
        return GLProgram::VarType::Sampler2DRectShadow;
    case GL_INT_SAMPLER_1D:
        return GLProgram::VarType::ISampler1D;
    case GL_INT_SAMPLER_2D:
        return GLProgram::VarType::ISampler2D;
    case GL_INT_SAMPLER_3D:
        return GLProgram::VarType::ISampler3D;
    case GL_INT_SAMPLER_CUBE:
        return GLProgram::VarType::ISamplerCubemap;
    case GL_INT_SAMPLER_1D_ARRAY:
        return GLProgram::VarType::ISampler1DArray;
    case GL_INT_SAMPLER_2D_ARRAY:
        return GLProgram::VarType::ISampler2DArray;
    case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
        return GLProgram::VarType::ISamplerCubemapArray;
    case GL_INT_SAMPLER_2D_MULTISAMPLE:
        return GLProgram::VarType::ISampler2DMultiSample;
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        return GLProgram::VarType::ISampler2DMultiSampleArray;
    case GL_INT_SAMPLER_BUFFER:
        return GLProgram::VarType::ISamplerBuffer;
    case GL_INT_SAMPLER_2D_RECT:
        return GLProgram::VarType::ISampler2DRect;
    case GL_UNSIGNED_INT_SAMPLER_1D:
        return GLProgram::VarType::USampler1D;
    case GL_UNSIGNED_INT_SAMPLER_2D:
        return GLProgram::VarType::USampler2D;
    case GL_UNSIGNED_INT_SAMPLER_3D:
        return GLProgram::VarType::USampler3D;
    case GL_UNSIGNED_INT_SAMPLER_CUBE:
        return GLProgram::VarType::USamplerCubemap;
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        return GLProgram::VarType::USampler1DArray;
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        return GLProgram::VarType::USampler2DArray;
    case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
        return GLProgram::VarType::USamplerCubemapArray;
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        return GLProgram::VarType::USampler2DMultiSample;
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        return GLProgram::VarType::USampler2DMultiSampleArray;
    case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        return GLProgram::VarType::USamplerBuffer;
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
        return GLProgram::VarType::USampler2DRect;
    case GL_IMAGE_1D:
        return GLProgram::VarType::Image1D;
    case GL_IMAGE_2D:
        return GLProgram::VarType::Image2D;
    case GL_IMAGE_3D:
        return GLProgram::VarType::Image3D;
    case GL_IMAGE_2D_RECT:
        return GLProgram::VarType::Image2DRect;
    case GL_IMAGE_CUBE:
        return GLProgram::VarType::ImageCubemap;
    case GL_IMAGE_BUFFER:
        return GLProgram::VarType::ImageBuffer;
    case GL_IMAGE_1D_ARRAY:
        return GLProgram::VarType::Image1DArray;
    case GL_IMAGE_2D_ARRAY:
        return GLProgram::VarType::Image2DArray;
    case GL_IMAGE_CUBE_MAP_ARRAY:
        return GLProgram::VarType::ImageCubemapArray;
    case GL_IMAGE_2D_MULTISAMPLE:
        return GLProgram::VarType::Image2DMultiSample;
    case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
        return GLProgram::VarType::Image2DMultiSampleArray;
    case GL_INT_IMAGE_1D:
        return GLProgram::VarType::IImage1D;
    case GL_INT_IMAGE_2D:
        return GLProgram::VarType::IImage2D;
    case GL_INT_IMAGE_3D:
        return GLProgram::VarType::IImage3D;
    case GL_INT_IMAGE_2D_RECT:
        return GLProgram::VarType::IImage2DRect;
    case GL_INT_IMAGE_CUBE:
        return GLProgram::VarType::IImageCubemap;
    case GL_INT_IMAGE_BUFFER:
        return GLProgram::VarType::IImageBuffer;
    case GL_INT_IMAGE_1D_ARRAY:
        return GLProgram::VarType::IImage1DArray;
    case GL_INT_IMAGE_2D_ARRAY:
        return GLProgram::VarType::IImage2DArray;
    case GL_INT_IMAGE_CUBE_MAP_ARRAY:
        return GLProgram::VarType::IImageCubemapArray;
    case GL_INT_IMAGE_2D_MULTISAMPLE:
        return GLProgram::VarType::IImage2DMultiSample;
    case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
        return GLProgram::VarType::IImage2DMultiSampleArray;
    case GL_UNSIGNED_INT_IMAGE_1D:
        return GLProgram::VarType::UImage1D;
    case GL_UNSIGNED_INT_IMAGE_2D:
        return GLProgram::VarType::UImage2D;
    case GL_UNSIGNED_INT_IMAGE_3D:
        return GLProgram::VarType::UImage3D;
    case GL_UNSIGNED_INT_IMAGE_2D_RECT:
        return GLProgram::VarType::UImage2DRect;
    case GL_UNSIGNED_INT_IMAGE_CUBE:
        return GLProgram::VarType::UImageCubemap;
    case GL_UNSIGNED_INT_IMAGE_BUFFER:
        return GLProgram::VarType::UImageBuffer;
    case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
        return GLProgram::VarType::UImage1DArray;
    case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
        return GLProgram::VarType::UImage2DArray;
    case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
        return GLProgram::VarType::UImageCubemapArray;
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
        return GLProgram::VarType::UImage2DMultiSample;
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
        return GLProgram::VarType::UImage2DMultiSampleArray;
    case GL_UNSIGNED_INT_ATOMIC_COUNTER:
        return GLProgram::VarType::AtomicCounter;
    }

    std::cerr << std::format("Unknown shader variable type: {:#x}\n", static_cast<unsigned int>(type));

    return GLProgram::VarType::Unknown;
}

GLProgram::GLProgram(std::vector<ShaderModule> const& shaderModules,
                     TransformFeedbackBufferMode bufferMode,
                     std::vector<std::string> const& transformFeedbackVaryings) {
    // create program and attach/link shader modules
    m_handle = glCreateProgram();
    checkGLErrors();
    if (m_handle == 0) {
        throw std::runtime_error("Create OpenGL program failure");
    }

    for (auto const& shaderModule : shaderModules) {
        attach(shaderModule);
    }

    applyTransformFeedbackVaryings(bufferMode, transformFeedbackVaryings);
    link();

    for (auto const& shaderModule : shaderModules) {
        detach(shaderModule);
    }

    queryResourceMetaInfos();
#ifndef NDEBUG
    // printResourceInfos();
#endif
}

GLProgram::~GLProgram() {
    if (isValid()) {
        glDeleteProgram(m_handle);
    }
}

GLProgram& GLProgram::operator=(GLProgram&& rhs) noexcept {
    if (this != &rhs) {
        if (isValid()) {
            glDeleteProgram(m_handle);
        }

        m_uniformVarInfos = std::move(rhs.m_uniformVarInfos);
        m_textureInfos = std::move(rhs.m_textureInfos);
        m_uniformBufferInfos = std::move(rhs.m_uniformBufferInfos);
        m_storageBufferInfos = std::move(rhs.m_storageBufferInfos);
        m_storageImageInfos = std::move(rhs.m_storageImageInfos);
        m_atomicCounterInfos = std::move(rhs.m_atomicCounterInfos);
        m_transformFeedbackInfo = std::move(rhs.m_transformFeedbackInfo);

        RHIResource::operator=(std::move(rhs));
    }

    return *this;
}

void GLProgram::use() const {
    glUseProgram(m_handle);
}

void GLProgram::unuse() {
    glUseProgram(0);
}

int GLProgram::getUniformVarLocation(std::string_view name) const {
    auto it = m_uniformVarInfos.find(name);
    if (it != m_uniformVarInfos.end()) {
        return it->second.location;
    }
    // Array elements and sampler uniforms are not stored in m_uniformVarInfos.
    return glGetUniformLocation(m_handle, std::string(name).c_str());
}

int GLProgram::getTextureBinding(std::string_view name) const {
    for (auto const& [tName, info] : m_textureInfos) {
        if (tName == name) {
            return info.binding;
        }
    }

    return -1;
}

void GLProgram::setUniform(int location, bool const& value) const {
    glProgramUniform1i(m_handle, location, static_cast<int>(value));
}

void GLProgram::setUniform(int location, glm::bvec2 const& value) const {
    glProgramUniform2i(m_handle, location, static_cast<int>(value.x), static_cast<int>(value.y));
}

void GLProgram::setUniform(int location, glm::bvec3 const& value) const {
    glProgramUniform3i(
        m_handle, location, static_cast<int>(value.x), static_cast<int>(value.y), static_cast<int>(value.z));
}

void GLProgram::setUniform(int location, glm::bvec4 const& value) const {
    glProgramUniform4i(m_handle,
                       location,
                       static_cast<int>(value.x),
                       static_cast<int>(value.y),
                       static_cast<int>(value.z),
                       static_cast<int>(value.w));
}

void GLProgram::setUniform(int location, int const& value) const {
    glProgramUniform1i(m_handle, location, value);
}

void GLProgram::setUniform(int location, glm::ivec2 const& value) const {
    glProgramUniform2iv(m_handle, location, 1, &value[0]);
}

void GLProgram::setUniform(int location, glm::ivec3 const& value) const {
    glProgramUniform3iv(m_handle, location, 1, &value[0]);
}

void GLProgram::setUniform(int location, glm::ivec4 const& value) const {
    glProgramUniform4iv(m_handle, location, 1, &value[0]);
}

void GLProgram::setUniform(int location, uint32_t const& value) const {
    glProgramUniform1ui(m_handle, location, value);
}

void GLProgram::setUniform(int location, glm::uvec2 const& value) const {
    glProgramUniform2uiv(m_handle, location, 1, &value[0]);
}

void GLProgram::setUniform(int location, glm::uvec3 const& value) const {
    glProgramUniform3uiv(m_handle, location, 1, &value[0]);
}

void GLProgram::setUniform(int location, glm::uvec4 const& value) const {
    glProgramUniform4uiv(m_handle, location, 1, &value[0]);
}

void GLProgram::setUniform(int location, float const& value) const {
    glProgramUniform1f(m_handle, location, value);
}

void GLProgram::setUniform(int location, glm::vec2 const& value) const {
    glProgramUniform2fv(m_handle, location, 1, &value[0]);
}

void GLProgram::setUniform(int location, glm::vec3 const& value) const {
    glProgramUniform3fv(m_handle, location, 1, &value[0]);
}

void GLProgram::setUniform(int location, glm::vec4 const& value) const {
    glProgramUniform4fv(m_handle, location, 1, &value[0]);
}

void GLProgram::setUniform(int location, double const& value) const {
    glProgramUniform1d(m_handle, location, value);
}

void GLProgram::setUniform(int location, glm::dvec2 const& value) const {
    glProgramUniform2dv(m_handle, location, 1, &value[0]);
}

void GLProgram::setUniform(int location, glm::dvec3 const& value) const {
    glProgramUniform3dv(m_handle, location, 1, &value[0]);
}

void GLProgram::setUniform(int location, glm::dvec4 const& value) const {
    glProgramUniform4dv(m_handle, location, 1, &value[0]);
}

void GLProgram::setUniform(int location, glm::mat2x2 const& value) const {
    glProgramUniformMatrix2fv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::mat2x3 const& value) const {
    glProgramUniformMatrix2x3fv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::mat2x4 const& value) const {
    glProgramUniformMatrix2x4fv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::mat3x2 const& value) const {
    glProgramUniformMatrix3x2fv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::mat3x3 const& value) const {
    glProgramUniformMatrix3fv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::mat3x4 const& value) const {
    glProgramUniformMatrix3x4fv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::mat4x2 const& value) const {
    glProgramUniformMatrix4x2fv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::mat4x3 const& value) const {
    glProgramUniformMatrix4x3fv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::mat4x4 const& value) const {
    glProgramUniformMatrix4fv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::dmat2x2 const& value) const {
    glProgramUniformMatrix2dv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::dmat2x3 const& value) const {
    glProgramUniformMatrix2x3dv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::dmat2x4 const& value) const {
    glProgramUniformMatrix2x4dv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::dmat3x2 const& value) const {
    glProgramUniformMatrix3x2dv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::dmat3x3 const& value) const {
    glProgramUniformMatrix3dv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::dmat3x4 const& value) const {
    glProgramUniformMatrix3x4dv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::dmat4x2 const& value) const {
    glProgramUniformMatrix4x2dv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::dmat4x3 const& value) const {
    glProgramUniformMatrix4x3dv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::setUniform(int location, glm::dmat4x4 const& value) const {
    glProgramUniformMatrix4dv(m_handle, location, 1, GL_FALSE, &value[0][0]);
}

void GLProgram::printResourceInfos() const {
    std::cout << "Resource Infos\n";
    printUniformVarInfos();
    printTextureInfos();
    printUniformBlockInfos();
    printStorageBufferInfos();
    printStorageImageInfos();
    printAtomicCounterInfos();
    printTransformFeedbackInfo();
}

void GLProgram::attach(ShaderModule const& shaderModule) {
    glAttachShader(m_handle, shaderModule.getNativeHandle());
}

void GLProgram::detach(ShaderModule const& shaderModule) {
    glDetachShader(m_handle, shaderModule.getNativeHandle());
}

void GLProgram::applyTransformFeedbackVaryings(TransformFeedbackBufferMode bufferMode,
                                               std::vector<std::string> const& transformFeedbackVaryings) {
    if (bufferMode == TransformFeedbackBufferMode::None || transformFeedbackVaryings.empty()) {
        return;
    }

    std::vector<char const*> names;
    names.reserve(transformFeedbackVaryings.size());
    for (auto const& name : transformFeedbackVaryings) {
        names.push_back(name.c_str());
    }
    glTransformFeedbackVaryings(m_handle,
                                static_cast<GLsizei>(names.size()),
                                names.data(),
                                bufferMode == TransformFeedbackBufferMode::Interleaved ? GL_INTERLEAVED_ATTRIBS
                                                                                       : GL_SEPARATE_ATTRIBS);
}

void GLProgram::link() {
    glLinkProgram(m_handle);

    GLint result = GL_FALSE;
    glGetProgramiv(m_handle, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        GLint infoLogLength{0};
        glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<char> buffer(infoLogLength);
        glGetProgramInfoLog(m_handle, infoLogLength, NULL, &buffer[0]);
        throw std::runtime_error("Link error: " + std::string(buffer.data()));
    }
}

void GLProgram::queryResourceMetaInfos() {
    // Every interesting reference
    // https://www.geometrictools.com/GTE/Graphics/GL45/GLSLReflection.cpp
    queryGenericUniforms();
    queryUniformBlocks();
    queryShaderStorageBuffers();
    queryAtomicCounters();
    queryTransformFeedbackInfo();
}

void GLProgram::queryTransformFeedbackInfo() {
    m_transformFeedbackInfo = {};
    GLint varyingCount{};
    glGetProgramiv(m_handle, GL_TRANSFORM_FEEDBACK_VARYINGS, &varyingCount);
    if (varyingCount == 0) {
        return;
    }

    GLint bufferMode{};
    glGetProgramiv(m_handle, GL_TRANSFORM_FEEDBACK_BUFFER_MODE, &bufferMode);
    m_transformFeedbackInfo.bufferMode = bufferMode == GL_SEPARATE_ATTRIBS ? TransformFeedbackBufferMode::Separate
                                                                           : TransformFeedbackBufferMode::Interleaved;

    GLint maxNameLength{};
    glGetProgramiv(m_handle, GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH, &maxNameLength);
    std::vector<GLchar> name(static_cast<size_t>(maxNameLength));
    m_transformFeedbackInfo.varyings.reserve(static_cast<size_t>(varyingCount));
    for (GLint i = 0; i < varyingCount; ++i) {
        GLsizei nameLength{};
        GLsizei size{};
        GLenum type{};
        glGetTransformFeedbackVarying(
            m_handle, static_cast<GLuint>(i), maxNameLength, &nameLength, &size, &type, name.data());
        m_transformFeedbackInfo.varyings.push_back(
            {std::string(name.data(), static_cast<size_t>(nameLength)),
             type == GL_NONE ? VarType::Unknown : getVarType(type),
             size});
    }
}

void GLProgram::queryGenericUniforms() {
    GLint uniformCount{};
    glGetProgramiv(m_handle, GL_ACTIVE_UNIFORMS, &uniformCount);

    GLint maxNameLength{};
    glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);
    std::vector<GLchar> name(maxNameLength);

    for (GLint i = 0; i < uniformCount; ++i) {
        GLsizei nameLength{};
        GLint arraySize{};
        GLenum type{};
        glGetActiveUniform(m_handle, i, maxNameLength, &nameLength, &arraySize, &type, name.data());

        // Skip uniforms in uniform blocks
        GLuint const uniformIndex = static_cast<GLuint>(i);
        GLint blockIndex{};
        glGetActiveUniformsiv(m_handle, 1, &uniformIndex, GL_UNIFORM_BLOCK_INDEX, &blockIndex);
        if (blockIndex != -1) {
            continue;
        }

        std::string uniformName(name.data(), static_cast<size_t>(nameLength));
        GLint const location = glGetUniformLocation(m_handle, uniformName.c_str());
        if (location < 0) {
            continue;
        }

        VarType varType = getVarType(type);
        if (varType == GLProgram::VarType::Unknown) {
            std::cerr << "Cannot get shader variable type of " << uniformName << '\n';
        }

        if (isTexture(varType)) {
            GLint binding;
            glGetUniformiv(m_handle, location, &binding);
            m_textureInfos.try_emplace(std::move(uniformName), binding, varType);
        }
        else if (isImage(varType)) {
            GLint binding;
            glGetUniformiv(m_handle, location, &binding);
            m_storageImageInfos.try_emplace(std::move(uniformName), binding, varType);
        }
        else {
            m_uniformVarInfos.try_emplace(std::move(uniformName), location, varType);
        }
    }

    // We reassign all texture binding if all textures bindings are 0
    // Since we use binding instead of location for textures
    if (!m_textureInfos.empty()) {
        bool allZero = true;
        for (auto const& [name, info] : m_textureInfos) {
            if (info.binding != 0) {
                allZero = false;
                break;
            }
        }

        if (allZero) {
            int nextBinding = 0;
            for (auto& [name, info] : m_textureInfos) {
                GLint const location = glGetUniformLocation(m_handle, name.c_str());
                if (location < 0) {
                    continue;
                }

                glProgramUniform1i(m_handle, location, nextBinding);
                info.binding = nextBinding;
                ++nextBinding;
            }
        }
    }
}

void GLProgram::queryUniformBlocks() {
    GLint blockCount = 0;
    glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount);

    GLint maxNameLength{};
    glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &maxNameLength);
    std::vector<GLchar> name(maxNameLength);

    for (GLint i = 0; i < blockCount; ++i) {
        GLsizei nameLength{};
        glGetActiveUniformBlockName(m_handle, i, maxNameLength, &nameLength, name.data());

        GLint binding{};
        glGetActiveUniformBlockiv(m_handle, i, GL_UNIFORM_BLOCK_BINDING, &binding);
        GLint size{};
        glGetActiveUniformBlockiv(m_handle, i, GL_UNIFORM_BLOCK_DATA_SIZE, &size);

        UniformBufferInfo uboInfo{
            .binding = binding,
            .size = static_cast<uint32_t>(size),
        };

        m_uniformBufferInfos.emplace(std::string(name.data(), static_cast<size_t>(nameLength)), uboInfo);
    }
}

void GLProgram::queryShaderStorageBuffers() {
    // Shader storage buffers require OpenGL 4.3.
}

void GLProgram::queryAtomicCounters() {
    // Atomic counters require OpenGL 4.2.
}

void GLProgram::printUniformVarInfos() const {
    std::cout << "+ Uniform Var Infos\n";
    for (auto const& [name, varInfo] : m_uniformVarInfos) {
        std::cout << "  + " << name << "\n";
        std::cout << "    + location: " << varInfo.location << "\n";
        std::cout << "    + type:     " << getVarTypeName(varInfo.type) << "\n";
    }
}

void GLProgram::printTextureInfos() const {
    std::cout << "+ Texture Infos\n";
    for (auto const& [name, texInfo] : m_textureInfos) {
        std::cout << "  + " << name << "\n";
        std::cout << "    + binding: " << texInfo.binding << "\n";
        std::cout << "    + type:    " << getVarTypeName(texInfo.type) << "\n";
    }
}

void GLProgram::printUniformBlockInfos() const {
    std::cout << "+ Uniform Block Infos\n";
    for (auto const& [name, uboInfo] : m_uniformBufferInfos) {
        std::cout << "  + " << name << "\n";
        std::cout << "    + binding: " << uboInfo.binding << "\n";
        std::cout << "    + size:    " << uboInfo.size << "\n";
    }
}

void GLProgram::printStorageBufferInfos() const {
    std::cout << "+ Storage Buffer Infos\n";
    for (auto const& [name, ssboInfo] : m_storageBufferInfos) {
        std::cout << "  + " << name << "\n";
        std::cout << "    + binding: " << ssboInfo.binding << "\n";
    }
}

void GLProgram::printStorageImageInfos() const {
    std::cout << "+ Storage Image Infos\n";
    for (auto const& [name, imageInfo] : m_storageImageInfos) {
        std::cout << "  + " << name << "\n";
        std::cout << "    + binding: " << imageInfo.binding << "\n";
        std::cout << "    + type:    " << getVarTypeName(imageInfo.type) << "\n";
    }
}

void GLProgram::printAtomicCounterInfos() const {
    std::cout << "+ Atomic Counter Infos\n";
    for (auto const& [name, atomicInfo] : m_atomicCounterInfos) {
        std::cout << "  + " << name << "\n";
        std::cout << "    + binding: " << atomicInfo.binding << "\n";
        std::cout << "    + offset:  " << atomicInfo.offset << "\n";
    }
}

void GLProgram::printTransformFeedbackInfo() const {
    std::cout << "+ Transform Feedback Info\n";
    std::cout << "  + buffer mode: ";
    switch (m_transformFeedbackInfo.bufferMode) {
    case TransformFeedbackBufferMode::None:
        std::cout << "None";
        break;
    case TransformFeedbackBufferMode::Interleaved:
        std::cout << "Interleaved";
        break;
    case TransformFeedbackBufferMode::Separate:
        std::cout << "Separate";
        break;
    }
    std::cout << '\n';
    for (auto const& info : m_transformFeedbackInfo.varyings) {
        std::cout << "  + " << info.name << "\n";
        std::cout << "    + type:       " << getVarTypeName(info.type) << "\n";
        std::cout << "    + size:       " << info.size << "\n";
    }
}

bool GLProgram::isTexture(VarType type) noexcept {
    switch (type) {
    default:
        break;
    case VarType::Sampler1D:
    case VarType::Sampler2D:
    case VarType::Sampler3D:
    case VarType::SamplerCubemap:
    case VarType::Sampler1DShadow:
    case VarType::Sampler2DShadow:
    case VarType::Sampler1DArray:
    case VarType::Sampler2DArray:
    case VarType::SamplerCubeArray:
    case VarType::Sampler1DArrayShadow:
    case VarType::Sampler2DArrayShadow:
    case VarType::Sampler2DMultiSample:
    case VarType::Sampler2DMultiSampleArray:
    case VarType::SamplerCubemapShadow:
    case VarType::SamplerCubemapArrayShadow:
    case VarType::SamplerBuffer:
    case VarType::Sampler2DRect:
    case VarType::Sampler2DRectShadow:
    case VarType::ISampler1D:
    case VarType::ISampler2D:
    case VarType::ISampler3D:
    case VarType::ISamplerCubemap:
    case VarType::ISampler1DArray:
    case VarType::ISampler2DArray:
    case VarType::ISamplerCubemapArray:
    case VarType::ISampler2DMultiSample:
    case VarType::ISampler2DMultiSampleArray:
    case VarType::ISamplerBuffer:
    case VarType::ISampler2DRect:
    case VarType::USampler1D:
    case VarType::USampler2D:
    case VarType::USampler3D:
    case VarType::USamplerCubemap:
    case VarType::USampler1DArray:
    case VarType::USampler2DArray:
    case VarType::USamplerCubemapArray:
    case VarType::USampler2DMultiSample:
    case VarType::USampler2DMultiSampleArray:
    case VarType::USamplerBuffer:
    case VarType::USampler2DRect:
        return true;
    }

    return false;
}

bool GLProgram::isImage(VarType type) noexcept {
    switch (type) {
    default:
        break;
    case VarType::Image1D:
    case VarType::Image2D:
    case VarType::Image3D:
    case VarType::Image2DRect:
    case VarType::ImageCubemap:
    case VarType::ImageBuffer:
    case VarType::Image1DArray:
    case VarType::Image2DArray:
    case VarType::ImageCubemapArray:
    case VarType::Image2DMultiSample:
    case VarType::Image2DMultiSampleArray:
    case VarType::IImage1D:
    case VarType::IImage2D:
    case VarType::IImage3D:
    case VarType::IImage2DRect:
    case VarType::IImageCubemap:
    case VarType::IImageBuffer:
    case VarType::IImage1DArray:
    case VarType::IImage2DArray:
    case VarType::IImageCubemapArray:
    case VarType::IImage2DMultiSample:
    case VarType::IImage2DMultiSampleArray:
    case VarType::UImage1D:
    case VarType::UImage2D:
    case VarType::UImage3D:
    case VarType::UImage2DRect:
    case VarType::UImageCubemap:
    case VarType::UImageBuffer:
    case VarType::UImage1DArray:
    case VarType::UImage2DArray:
    case VarType::UImageCubemapArray:
    case VarType::UImage2DMultiSample:
    case VarType::UImage2DMultiSampleArray:
        return true;
    }

    return false;
}
} // namespace gfx
