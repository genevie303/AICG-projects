#pragma once

#include <memory>
#include <vector>

#include <common/application.h>
#include <graphics/gl_program.h>
#include <graphics/sampler.h>
#include <graphics/vertex_array.h>
#include <scene/camera/camera.h>

#include "cpu_ray_tracer.h"
#include "gpu_ray_tracer.h"
#include "material.h"
#include "model.h"
#include "sphere.h"

class RayTracerApp final : public Application {
public:
    explicit RayTracerApp(Config const& config);

    ~RayTracerApp() override;

private:
    // components to compose the scene
    std::unique_ptr<Model> m_lucy;
    std::vector<Sphere> m_spheres;
    std::vector<Material> m_sphereMaterials;

    int m_renderSceneIndex{0};
    int m_activeSceneIndex{-1};
    int m_rendererIndex{0};

    // camera
    PerspectiveCamera m_camera{glm::radians(60.0f), m_window.aspectRatio(), 0.1f, 1000.0f};

    // raytracers
    std::unique_ptr<CpuRayTracer> m_cpuTracer;
    std::unique_ptr<GpuRayTracer> m_gpuTracer;

    // presentation
    std::unique_ptr<gfx::VertexArray> m_screenQuad;
    std::unique_ptr<gfx::GLProgram> m_presentProgram;
    std::unique_ptr<gfx::Sampler> m_presentSampler;

private:
    void handleEvent(Event& event) override;

    void renderFrame() override;

    void updateCamera();

    void createSpheres();

    void createRenderScene(int index);

    void createScene1();

    void createScene2();

    void createScene3();

    void createTracerScene(std::vector<Sphere> const& spheres,
                           std::vector<Model const*> const& models,
                           std::vector<glm::mat4> const& transforms,
                           std::vector<Material> const& sphereMaterials,
                           std::vector<Material> const& modelMaterials);

    void drawScreenQuad() const;

    void initUI();

    void renderUI();

    void resetAccumulation() noexcept;
};
