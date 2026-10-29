#pragma once

#include <cstdint>
#include <memory>

#include <glm/glm.hpp>

#include <graphics/texture/texture2d.h>

#include "scene.h"
#include "skybox.h"

class RayTracer {
public:
    RayTracer(uint32_t width, uint32_t height);

    virtual ~RayTracer() = default;

    virtual void setScene(std::shared_ptr<Scene const> scene);

    void setSkybox(std::shared_ptr<Skybox const> skybox);

    virtual void resize(uint32_t width, uint32_t height) = 0;

    virtual void reset();

    virtual void render(glm::mat4 const& cameraToWorld, glm::mat4 const& rasterToCamera) = 0;

    virtual gfx::Texture2D const& frame() const = 0;

    uint32_t completedSamples() const noexcept {
        return m_completedSamples;
    }

    uint32_t width() const noexcept {
        return m_width;
    }

    uint32_t height() const noexcept {
        return m_height;
    }

protected:
    std::shared_ptr<Scene const> m_scene;
    std::shared_ptr<Skybox const> m_skybox;

    uint32_t m_width{0};
    uint32_t m_height{0};
    uint32_t m_completedSamples{0};

    bool updateExtent(uint32_t width, uint32_t height) noexcept;
};
