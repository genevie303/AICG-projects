#include "ray_tracer.h"

#include <utility>

RayTracer::RayTracer(uint32_t width, uint32_t height)
    : m_width{width}
    , m_height{height} {}

void RayTracer::setScene(std::shared_ptr<Scene const> scene) {
    m_scene = std::move(scene);
    reset();
}

void RayTracer::setSkybox(std::shared_ptr<Skybox const> skybox) {
    m_skybox = std::move(skybox);
    reset();
}

bool RayTracer::updateExtent(uint32_t width, uint32_t height) noexcept {
    if (m_width == width && m_height == height) {
        return false;
    }
    m_width = width;
    m_height = height;
    return true;
}

void RayTracer::reset() {
    m_completedSamples = 0;
}
