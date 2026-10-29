#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "material.h"
#include "primitive.h"
#include "ray.h"
#include "ray_tracer.h"

class CpuRayTracer final : public RayTracer {
public:
    CpuRayTracer(uint32_t width, uint32_t height);

    void render(glm::mat4 const& cameraToWorld, glm::mat4 const& rasterToCamera) override;

    void resize(uint32_t width, uint32_t height) override;

    void reset() override;

    gfx::Texture2D const& frame() const override {
        return *m_frame;
    }

private:
    struct Hit {
        Primitive primitive{};
        glm::vec3 position{0.0f};
        glm::vec3 normal{0.0f};
        uint32_t materialIndex{0};
    };

private:
    std::vector<glm::vec3> m_average;
    std::vector<uint32_t> m_rngStates;

    std::unique_ptr<gfx::Texture2D> m_frame;

private:
    void createFrameResources();

    bool intersect(Ray& ray, Hit& hit) const;

    bool intersectPrimitive(Ray& ray, Primitive const& primitive, Hit& hit) const;

    bool lambertianScatterFunction(Ray& ray, Hit const& hit, Material const& material, uint32_t& rngState) const;

    bool metalScatterFunction(Ray& ray, Hit const& hit, Material const& material, uint32_t& rngState) const;

    void dielectricScatterFunction(Ray& ray, Hit const& hit, Material const& material, uint32_t& rngState) const;

    glm::vec3 trace(Ray ray, uint32_t& rngState) const;
};
