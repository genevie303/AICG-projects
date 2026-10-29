#include "gpu_ray_tracer.h"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

#include <common/file_utils.h>
#include <graphics/rhi.h>

template <typename T>
static std::unique_ptr<gfx::Texture2D> makeDataTexture(std::vector<T>& data,
                                                       uint32_t components,
                                                       gfx::Texture::Format format,
                                                       gfx::Texture::ExternalFormat pixelFormat,
                                                       gfx::Texture::PixelType pixelType) {
    constexpr uint32_t width = 2048;
    auto const rowSize = static_cast<size_t>(width) * components;
    auto const height = std::max(size_t{1}, (data.size() + rowSize - 1) / rowSize);
    data.resize(height * rowSize, T{});
    return std::make_unique<gfx::Texture2D>(
        gfx::Texture2D::Specification{
            .width = width,
            .height = static_cast<uint32_t>(height),
            .format = format,
            .pixelFormat = pixelFormat,
            .pixelType = pixelType
        },
        data.data());
}

GpuRayTracer::GpuRayTracer(uint32_t width,
                           uint32_t height,
                           std::filesystem::path const& vertexShader,
                           std::filesystem::path const& fragmentShader)
    : RayTracer{width, height} {
    std::vector<gfx::ShaderModule> modules;
    modules.emplace_back(readTextFile(vertexShader), gfx::ShaderModule::Stage::Vertex);
    modules.emplace_back(readTextFile(fragmentShader), gfx::ShaderModule::Stage::Fragment);
    m_program = std::make_unique<gfx::GLProgram>(modules);
    m_screenQuad = std::make_unique<gfx::VertexArray>();
    m_nearestSampler = std::make_unique<gfx::Sampler>(gfx::Sampler::Description{
        .addressModeU = gfx::Sampler::AddressMode::ClampToEdge,
        .addressModeV = gfx::Sampler::AddressMode::ClampToEdge,
        .addressModeW = gfx::Sampler::AddressMode::ClampToEdge
    });
    m_skyboxSampler = std::make_unique<gfx::Sampler>(gfx::Sampler::Description{
        .minFilter = gfx::Sampler::Filter::Linear,
        .magFilter = gfx::Sampler::Filter::Linear,
        .addressModeU = gfx::Sampler::AddressMode::ClampToEdge,
        .addressModeV = gfx::Sampler::AddressMode::ClampToEdge,
        .addressModeW = gfx::Sampler::AddressMode::ClampToEdge
    });
    createRenderTargets();
    reset();
}

void GpuRayTracer::resize(uint32_t width, uint32_t height) {
    if (!updateExtent(width, height)) {
        return;
    }
    createRenderTargets();
    reset();
}

void GpuRayTracer::createRenderTargets() {
    std::vector<uint32_t> rng(static_cast<size_t>(m_width) * m_height);
    for (uint32_t i = 0; i < rng.size(); ++i) {
        rng[i] = 1664525u * i + 1013904223u;
    }
    std::vector<glm::vec4> emptyFrame(static_cast<size_t>(m_width) * m_height, glm::vec4{0.0f});
    for (size_t i = 0; i < 2; ++i) {
        m_sampleFramebuffers[i] = std::make_unique<gfx::Framebuffer>();
        m_outFrames[i] = std::make_unique<gfx::Texture2D>(
            gfx::Texture2D::Specification{
                .width = m_width,
                .height = m_height,
                .format = gfx::Texture::Format::RGBA32F,
                .pixelFormat = gfx::Texture::ExternalFormat::RGBA,
                .pixelType = gfx::Texture::PixelType::Float
            },
            emptyFrame.data());
        m_rngStates[i] = std::make_unique<gfx::Texture2D>(
            gfx::Texture2D::Specification{
                .width = m_width,
                .height = m_height,
                .format = gfx::Texture::Format::R32UI,
                .pixelFormat = gfx::Texture::ExternalFormat::RedInt,
                .pixelType = gfx::Texture::PixelType::UInt
            },
            rng.data());
        m_sampleFramebuffers[i]->attachAsColor(0, *m_outFrames[i]);
        m_sampleFramebuffers[i]->attachAsColor(1, *m_rngStates[i]);
        m_sampleFramebuffers[i]->setColorWrites({0, 1});
        if (!m_sampleFramebuffers[i]->isComplete()) {
            throw std::runtime_error(m_sampleFramebuffers[i]->getDiagnostic());
        }
    }

    m_currentReadBuffer = 0;
    m_currentWriteBuffer = 1;
}

void GpuRayTracer::setScene(std::shared_ptr<Scene const> scene) {
    RayTracer::setScene(std::move(scene));
    uploadSceneData();
}

void GpuRayTracer::uploadSceneData() {
    std::vector<float> sphereData;
    sphereData.reserve(m_scene->spheres().size() * 4);
    for (auto const& sphere : m_scene->spheres()) {
        sphereData.insert(sphereData.end(), {sphere.position.x, sphere.position.y, sphere.position.z, sphere.radius});
    }
    m_sphereBuffer = makeDataTexture(sphereData,
                                     4,
                                     gfx::Texture::Format::RGBA32F,
                                     gfx::Texture::ExternalFormat::RGBA,
                                     gfx::Texture::PixelType::Float);

    // Explicit packing matches getTriangleData in the shader regardless of GLM alignment.
    std::vector<float> vertexData;
    vertexData.reserve(m_scene->vertices().size() * 8);
    for (auto const& vertex : m_scene->vertices()) {
        vertexData.insert(vertexData.end(),
                          {vertex.position.x,
                           vertex.position.y,
                           vertex.position.z,
                           vertex.normal.x,
                           vertex.normal.y,
                           vertex.normal.z,
                           vertex.texCoord.x,
                           vertex.texCoord.y});
    }
    m_vertexBuffer = makeDataTexture(vertexData,
                                     4,
                                     gfx::Texture::Format::RGBA32F,
                                     gfx::Texture::ExternalFormat::RGBA,
                                     gfx::Texture::PixelType::Float);

    std::vector<int32_t> triangleData;
    triangleData.reserve(m_scene->triangles().size() * 3);
    for (auto const& triangle : m_scene->triangles()) {
        for (auto index : triangle.v) {
            triangleData.push_back(static_cast<int32_t>(index));
        }
    }
    m_indexBuffer = makeDataTexture(triangleData,
                                    3,
                                    gfx::Texture::Format::RGB32I,
                                    gfx::Texture::ExternalFormat::RGBInt,
                                    gfx::Texture::PixelType::Int);

    std::vector<float> materialData;
    materialData.reserve(m_scene->materials().size() * 6);
    for (auto const& material : m_scene->materials()) {
        materialData.insert(materialData.end(),
                            {static_cast<float>(material.type),
                             material.ior,
                             material.fuzz,
                             material.albedo.x,
                             material.albedo.y,
                             material.albedo.z});
    }
    m_materialBuffer = makeDataTexture(materialData,
                                       3,
                                       gfx::Texture::Format::RGB32F,
                                       gfx::Texture::ExternalFormat::RGB,
                                       gfx::Texture::PixelType::Float);

    std::vector<int32_t> primitiveData;
    primitiveData.reserve(m_scene->primitives().size() * 3);
    for (auto const& primitive : m_scene->primitives()) {
        primitiveData.insert(primitiveData.end(),
                             {static_cast<int32_t>(primitive.type),
                              static_cast<int32_t>(primitive.shapeIndex),
                              static_cast<int32_t>(primitive.materialIndex)});
    }
    m_primitiveBuffer = makeDataTexture(primitiveData,
                                        3,
                                        gfx::Texture::Format::RGB32I,
                                        gfx::Texture::ExternalFormat::RGBInt,
                                        gfx::Texture::PixelType::Int);

    std::vector<float> bvhData;
    bvhData.reserve(m_scene->nodes().size() * 8);
    for (auto const& node : m_scene->nodes()) {
        // Two RGBA texels: min.xyz/max.x, then max.yz/first/second.
        bvhData.insert(bvhData.end(),
                       {node.bounds.min.x,
                        node.bounds.min.y,
                        node.bounds.min.z,
                        node.bounds.max.x,
                        node.bounds.max.y,
                        node.bounds.max.z,
                        static_cast<float>(node.first),
                        static_cast<float>(node.second)});
    }
    m_bvhBuffer = makeDataTexture(
        bvhData, 4, gfx::Texture::Format::RGBA32F, gfx::Texture::ExternalFormat::RGBA, gfx::Texture::PixelType::Float);

    m_program->setUniform("nPrimitives", static_cast<int>(m_scene->primitives().size()));
    m_program->setUniform("nBVHNodes", static_cast<int>(m_scene->nodes().size()));
}

void GpuRayTracer::render(glm::mat4 const& cameraToWorld, glm::mat4 const& rasterToCamera) {
    m_program->use();
    m_sampleFramebuffers[m_currentWriteBuffer]->bind();
    m_program->setUniform("totalSamples", m_completedSamples);
    m_program->setUniform("camera.cameraToWorld", cameraToWorld);
    m_program->setUniform("camera.rasterToCamera", rasterToCamera);
    bindSceneTextures();
    drawScreenQuad();
    gfx::Framebuffer::unbind();
    ++m_completedSamples;
    std::swap(m_currentReadBuffer, m_currentWriteBuffer);
}

void GpuRayTracer::bindSceneTextures() {
    auto bind = [this](gfx::Texture const& texture, char const* name, gfx::Sampler const& sampler) {
        auto const unit = m_program->getTextureBinding(name);
        if (unit < 0) {
            return;
        }

        texture.bind(unit);
        sampler.bind(unit);
    };

    bind(m_skybox->texture(), "sky", *m_skyboxSampler);
    bind(*m_sphereBuffer, "sphereBuffer", *m_nearestSampler);
    bind(*m_materialBuffer, "materialBuffer", *m_nearestSampler);
    bind(*m_primitiveBuffer, "primitiveBuffer", *m_nearestSampler);
    bind(*m_outFrames[m_currentReadBuffer], "RTResult", *m_nearestSampler);
    bind(*m_rngStates[m_currentReadBuffer], "oldRngState", *m_nearestSampler);
    bind(*m_indexBuffer, "triangleIndexBuffer", *m_nearestSampler);
    bind(*m_vertexBuffer, "vertexBuffer", *m_nearestSampler);
    bind(*m_bvhBuffer, "bvh", *m_nearestSampler);
}

void GpuRayTracer::drawScreenQuad() const {
    m_screenQuad->bind();
    gfx::RHI::draw(gfx::RHI::Primitive::Triangles, 0, 3);
    gfx::VertexArray::unbind();
}
