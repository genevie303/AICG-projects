#include "gpu_renderer.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <common/file_utils.h>
#include <graphics/rhi.h>
#include <graphics/shader_module.h>

GpuRenderer::GpuRenderer(uint32_t width,
                         uint32_t height,
                         std::filesystem::path const& transformShader,
                         std::filesystem::path const& rasterVertexShader,
                         std::filesystem::path const& rasterGeometryShader,
                         std::filesystem::path const& rasterFragmentShader)
    : Renderer(width, height) {
    /* load transform feedback shaders */ {
        std::vector<gfx::ShaderModule> shaderModules;
        shaderModules.emplace_back(readTextFile(transformShader), gfx::ShaderModule::Stage::Vertex);
        m_transformProgram =
            std::make_unique<gfx::GLProgram>(shaderModules,
                                             gfx::GLProgram::TransformFeedbackBufferMode::Interleaved,
                                             std::vector<std::string>{"tfClip", "tfWorldPosition", "tfNormal", "tfUv"});
    }

    /* load transform feedback shaders */ {
        std::vector<gfx::ShaderModule> modules;
        modules.emplace_back(readTextFile(rasterVertexShader), gfx::ShaderModule::Stage::Vertex);
        modules.emplace_back(readTextFile(rasterGeometryShader), gfx::ShaderModule::Stage::Geometry);
        modules.emplace_back(readTextFile(rasterFragmentShader), gfx::ShaderModule::Stage::Fragment);
        m_rasterProgram = std::make_unique<gfx::GLProgram>(modules);
        for (int i = 0; i < 5; ++i) {
            std::string const sampler = "uTextures[" + std::to_string(i) + "]";
            m_rasterProgram->setUniform(sampler, i);
        }
    }

    resizeBackend();
}

void GpuRenderer::resizeBackend() {
    m_depthBuffer = std::make_unique<gfx::Renderbuffer>(m_extent.x, m_extent.y, gfx::Renderbuffer::Format::Depth32F);
    m_framebuffer = std::make_unique<gfx::Framebuffer>();
    m_framebuffer->attachAsColor(0, *m_outputTexture);
    m_framebuffer->attachAsDepth(*m_depthBuffer);
    m_framebuffer->setColorWrite(0);
    if (!m_framebuffer->isComplete()) {
        throw std::runtime_error("GPU rasterizer framebuffer is incomplete");
    }
}

void GpuRenderer::transformVertices(Mesh const& mesh, FrameState const& frame) {
    auto const& indices = mesh.indices();
    size_t const outputBytes = indices.size() * sizeof(ClipVertex);
    auto const& inputVao = mesh.gpuVertexArray();
    if (!m_transformedBuffer) {
        m_transformedBuffer = std::make_unique<gfx::Buffer>(
            gfx::Buffer::UsageFlags::TransformFeedbackBuffer, outputBytes, gfx::Buffer::Usage::DynamicDraw);
        m_feedbackVao.setFloatAttribute(*m_transformedBuffer,
                                        {0,
                                         4,
                                         gfx::VertexArray::DataType::Float32,
                                         sizeof(ClipVertex),
                                         static_cast<intptr_t>(offsetof(ClipVertex, clip))});
        m_feedbackVao.setFloatAttribute(*m_transformedBuffer,
                                        {1,
                                         3,
                                         gfx::VertexArray::DataType::Float32,
                                         sizeof(ClipVertex),
                                         static_cast<intptr_t>(offsetof(ClipVertex, worldPosition))});
        m_feedbackVao.setFloatAttribute(*m_transformedBuffer,
                                        {2,
                                         3,
                                         gfx::VertexArray::DataType::Float32,
                                         sizeof(ClipVertex),
                                         static_cast<intptr_t>(offsetof(ClipVertex, normal))});
        m_feedbackVao.setFloatAttribute(*m_transformedBuffer,
                                        {3,
                                         2,
                                         gfx::VertexArray::DataType::Float32,
                                         sizeof(ClipVertex),
                                         static_cast<intptr_t>(offsetof(ClipVertex, uv))});
    }
    else if (m_transformedBuffer->getSize() != outputBytes) {
        m_transformedBuffer->resize(outputBytes, gfx::Buffer::Usage::DynamicDraw);
    }

    m_transformProgram->use();
    m_transformProgram->setUniform("uModel", frame.model);
    m_transformProgram->setUniform("uModelViewProjection", frame.projection * frame.view * frame.model);
    m_transformProgram->setUniform("uNormalTransform", glm::transpose(glm::inverse(glm::mat3(frame.model))));

    m_transformFeedback.setBuffer(0, *m_transformedBuffer);
    gfx::RHI::enableRasterizerDiscard(true);
    inputVao.bind();

    m_transformFeedback.begin(gfx::RHI::Primitive::Triangles);
    gfx::RHI::drawIndexed(gfx::RHI::Primitive::Triangles, 0, static_cast<uint32_t>(indices.size()));
    gfx::TransformFeedback::end();

    gfx::RHI::enableRasterizerDiscard(false);
    gfx::TransformFeedback::unbind();
    gfx::VertexArray::unbind();
}

void GpuRenderer::setMaterial(PbrMaterial const& material) {
    for (int i = 0; i < 5; ++i) {
        if (material.hasTexture[i]) {
            material.textures[i].gpuTexture->bind(i);
            material.textures[i].gpuSampler->bind(i);
        }

        m_rasterProgram->setUniform("uHasTexture[" + std::to_string(i) + "]", material.hasTexture[i]);
    }
    m_rasterProgram->setUniform("uBaseColorFactor", material.baseColorFactor);
    m_rasterProgram->setUniform("uEmissiveFactor", material.emissiveFactor);
    m_rasterProgram->setUniform("uMetallicFactor", material.metallicFactor);
    m_rasterProgram->setUniform("uRoughnessFactor", material.roughnessFactor);
    m_rasterProgram->setUniform("uOcclusionStrength", material.occlusionStrength);
}

void GpuRenderer::clear() {
    m_framebuffer->bind();
    gfx::RHI::enableDepthWrite(true);
    gfx::RHI::setClearColor(m_backgroundColor);
    gfx::RHI::clear(true, true);
    gfx::Framebuffer::unbind();
}

void GpuRenderer::render(DrawItem const& drawItem, FrameState const& frame) {
    m_framebuffer->bind();
    gfx::RHI::setViewport({m_extent.x, m_extent.y});
    gfx::RHI::enableDepthTest(true);

    transformVertices(*drawItem.mesh, frame);

    m_rasterProgram->use();
    m_rasterProgram->setUniform("uExtent", glm::vec2(frame.extent));
    m_rasterProgram->setUniform("uCameraPosition", frame.cameraPosition);
    m_rasterProgram->setUniform("uLightDirection", frame.directionalLight.transform.getFront());
    m_rasterProgram->setUniform("uLightColor", frame.directionalLight.color);
    m_rasterProgram->setUniform("uLightIntensity", frame.directionalLight.intensity);
    setMaterial(*drawItem.material);
    m_feedbackVao.bind();
    m_transformFeedback.draw(gfx::RHI::Primitive::Triangles);
    gfx::VertexArray::unbind();

    for (uint32_t unit = 0; unit < 5; ++unit) {
        gfx::Sampler::unbind(unit);
    }
    gfx::Framebuffer::unbind();
}
