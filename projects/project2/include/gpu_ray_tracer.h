#pragma once

#include <filesystem>
#include <memory>

#include <graphics/framebuffer.h>
#include <graphics/gl_program.h>
#include <graphics/sampler.h>
#include <graphics/vertex_array.h>

#include "ray_tracer.h"

class GpuRayTracer final : public RayTracer {
public:
    GpuRayTracer(uint32_t width,
                 uint32_t height,
                 std::filesystem::path const& vertexShader,
                 std::filesystem::path const& fragmentShader);

    void render(glm::mat4 const& cameraToWorld, glm::mat4 const& rasterToCamera) override;

    void setScene(std::shared_ptr<Scene const> scene) override;

    void resize(uint32_t width, uint32_t height) override;

    gfx::Texture2D const& frame() const override {
        return *m_outFrames[m_currentReadBuffer];
    }

private:
    std::unique_ptr<gfx::GLProgram> m_program;
    std::unique_ptr<gfx::VertexArray> m_screenQuad;
    std::unique_ptr<gfx::Sampler> m_nearestSampler;
    std::unique_ptr<gfx::Sampler> m_skyboxSampler;

    std::unique_ptr<gfx::Framebuffer> m_sampleFramebuffers[2];
    std::unique_ptr<gfx::Texture2D> m_outFrames[2];
    std::unique_ptr<gfx::Texture2D> m_rngStates[2];
    uint32_t m_currentReadBuffer{0};
    uint32_t m_currentWriteBuffer{1};

    std::unique_ptr<gfx::Texture2D> m_vertexBuffer;
    std::unique_ptr<gfx::Texture2D> m_indexBuffer;
    std::unique_ptr<gfx::Texture2D> m_sphereBuffer;
    std::unique_ptr<gfx::Texture2D> m_primitiveBuffer;
    std::unique_ptr<gfx::Texture2D> m_materialBuffer;
    std::unique_ptr<gfx::Texture2D> m_bvhBuffer;

private:
    void uploadSceneData();

    void createRenderTargets();

    void bindSceneTextures();

    void drawScreenQuad() const;
};
