#include "ray_tracer_app.h"
#include "random.h"

#include <array>
#include <cmath>
#include <format>
#include <numbers>
#include <stdexcept>
#include <utility>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <common/event/event_dispatcher.h>
#include <common/event/keyboard_event.h>
#include <common/file_utils.h>
#include <glm/ext.hpp>
#include <graphics/rhi.h>

RayTracerApp::RayTracerApp(Config const& config)
    : Application{config} {
    // models and spheres
    m_lucy = std::make_unique<Model>(getAssetPath("models/lucy.obj"));
    createSpheres();

    // skybox
    auto skybox = std::make_shared<Skybox>(std::array<std::filesystem::path, 6>{
        getAssetPath("skyboxrt/right.jpg"),
        getAssetPath("skyboxrt/left.jpg"),
        getAssetPath("skyboxrt/top.jpg"),
        getAssetPath("skyboxrt/bottom.jpg"),
        getAssetPath("skyboxrt/front.jpg"),
        getAssetPath("skyboxrt/back.jpg"),
    });

    // ray tracers
    auto const width = m_window.framebufferWidth();
    auto const height = m_window.framebufferHeight();
    m_cpuTracer = std::make_unique<CpuRayTracer>(width, height);
    m_cpuTracer->setSkybox(skybox);

    m_gpuTracer = std::make_unique<GpuRayTracer>(
        width, height, getAssetPath("shaders/quad.vert"), getAssetPath("shaders/raytracing.frag"));
    m_gpuTracer->setSkybox(std::move(skybox));

    // scene
    createRenderScene(m_renderSceneIndex);

    // utilities to present the rendered image to the screen
    m_screenQuad = std::make_unique<gfx::VertexArray>();
    m_presentSampler = std::make_unique<gfx::Sampler>(gfx::Sampler::Description{
        .addressModeU = gfx::Sampler::AddressMode::ClampToEdge,
        .addressModeV = gfx::Sampler::AddressMode::ClampToEdge,
        .addressModeW = gfx::Sampler::AddressMode::ClampToEdge
    });

    std::vector<gfx::ShaderModule> presentShaders;
    presentShaders.emplace_back(readTextFile(getAssetPath("shaders/quad.vert")), gfx::ShaderModule::Stage::Vertex);
    presentShaders.emplace_back(readTextFile(getAssetPath("shaders/quad.frag")), gfx::ShaderModule::Stage::Fragment);
    m_presentProgram = std::make_unique<gfx::GLProgram>(presentShaders);

    // ui
    initUI();

    // reset clock
    resetClock();
}

RayTracerApp::~RayTracerApp() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void RayTracerApp::handleEvent(Event& event) {
    EventDispatcher dispatcher{event};
    dispatcher.dispatch<KeyPressEvent>([this](KeyPressEvent& key) {
        if (key.getKeyCode() == KeyCode::Escape) {
            close();
            return true;
        }
        return false;
    });
}

void RayTracerApp::renderFrame() {
    if (m_activeSceneIndex != m_renderSceneIndex) {
        createRenderScene(m_renderSceneIndex);
    }

    updateCamera();
    auto const w = m_window.framebufferWidth(), h = m_window.framebufferHeight();
    if (w == 0 || h == 0) {
        return;
    }

    if (m_gpuTracer->width() != w || m_gpuTracer->height() != h) {
        m_gpuTracer->resize(w, h);
        m_cpuTracer->resize(w, h);
    }
    m_camera.aspect = m_window.aspectRatio();

    gfx::RHI::setViewport({w, h});
    gfx::RHI::enableDepthTest(false);
    gfx::RHI::enableBlend(false);
    gfx::RHI::enableScissorTest(false);
    gfx::RHI::enableCullFace(false);
    auto const screenToRaster =
        glm::scale(glm::mat4{1}, glm::vec3{.5f * w, .5f * h, 1}) * glm::translate(glm::mat4{1}, glm::vec3{1, 1, 0});
    auto const cameraToWorld = glm::inverse(m_camera.getViewMatrix());
    auto const rasterToCamera = glm::inverse(m_camera.getProjectionMatrix()) * glm::inverse(screenToRaster);
    RayTracer& tracer =
        m_rendererIndex == 0 ? static_cast<RayTracer&>(*m_gpuTracer) : static_cast<RayTracer&>(*m_cpuTracer);
    tracer.render(cameraToWorld, rasterToCamera);

    m_presentProgram->use();
    auto const frameUnit = m_presentProgram->getTextureBinding("frame");
    if (frameUnit < 0) {
        throw std::runtime_error("The frame sampler is not active.");
    }
    tracer.frame().bind(frameUnit);
    m_presentSampler->bind(frameUnit);
    drawScreenQuad();

    m_window.setTitle(std::format("Ray Tracer | samples: {}", tracer.completedSamples()));
    renderUI();
}

void RayTracerApp::updateCamera() {
    auto const step = 5.0f * getDeltaTime();
    auto const forward = m_camera.transform.getFront();
    auto const right = m_camera.transform.getRight();
    bool moved{};
    if (m_input.isKeyDown(KeyCode::W)) {
        m_camera.transform.position += forward * step;
        moved = true;
    }
    if (m_input.isKeyDown(KeyCode::S)) {
        m_camera.transform.position -= forward * step;
        moved = true;
    }
    if (m_input.isKeyDown(KeyCode::A)) {
        m_camera.transform.position -= right * step;
        moved = true;
    }
    if (m_input.isKeyDown(KeyCode::D)) {
        m_camera.transform.position += right * step;
        moved = true;
    }
    if (m_input.isKeyDown(KeyCode::Q)) {
        m_camera.transform.position += m_camera.transform.getUp() * step;
        moved = true;
    }
    if (m_input.isKeyDown(KeyCode::E)) {
        m_camera.transform.position -= m_camera.transform.getUp() * step;
        moved = true;
    }
    if (moved) {
        resetAccumulation();
    }
}

void RayTracerApp::createSpheres() {
    // use large ground sphere to approximate the ground
    m_spheres.emplace_back(glm::vec3{0.0f, -2000.0f, 0.0f}, 2000.0f);
    m_sphereMaterials.emplace_back(Material::Type::Lambertian, 1.0f, 0.0f, glm::vec3{0.5f});

    // small spheres
    constexpr float maxRadius = 15.0f;
    constexpr int radiusSegments = 100;
    constexpr int angleSegments = 60;
    for (int i = 0; i <= radiusSegments; i += 3) {
        for (int j = 0; j <= angleSegments; j += 2) {
            auto const rho = maxRadius * i / radiusSegments;
            auto const theta = 2 * std::numbers::pi_v<float> * j / angleSegments + 0.2f * randomFloat();
            auto const y = randomFloat(0.08f, 0.25f);
            glm::vec3 const center{rho * std::cos(theta), y, rho * std::sin(theta)};

            if (glm::length(center - glm::vec3{0.0f, 0.2f, 0.0f}) <= 2.0f ||
                glm::length(center - glm::vec3{4.0f, 0.2f, -2.0f}) <= 2.0f ||
                glm::length(center - glm::vec3{-4.0f, 0.2f, 2.0f}) <= 2.0f ||
                glm::length(center - glm::vec3{4.0f, 0.0f, 5.0f}) <= 1.0f) {
                continue;
            }

            Material material;
            auto const chooseMat = randomFloat();
            if (chooseMat < 0.4f) {
                material = {Material::Type::Lambertian, 1.0f, 0.0f, randomVec3() * randomVec3()};
            }
            else if (chooseMat < 0.95f) {
                material.type = Material::Type::Metal;
                material.ior = 1.0f;
                material.fuzz = randomFloat(0.0f, 0.5f);
                material.albedo = randomVec3(0.5f, 1.0f);
            }
            else {
                material = {Material::Type::Dielectric, 1.5f, 0.0f, glm::vec3{1.0f}};
            }

            m_spheres.emplace_back(center, center.y);
            m_sphereMaterials.push_back(material);
        }
    }

    // three big spheres
    m_spheres.emplace_back(glm::vec3{3.7f, 1.2f, 4.7f}, 1.2f);
    m_sphereMaterials.emplace_back(Material::Type::Dielectric, 1.5f, 0.0f, glm::vec3{1.0f});
    m_spheres.emplace_back(glm::vec3{-8.0f, 1.0f, -2.0f}, 1.0f);
    m_sphereMaterials.emplace_back(Material::Type::Lambertian, 1.0f, 0.0f, glm::vec3{0.2f, 0.4f, 0.8f});
    m_spheres.emplace_back(glm::vec3{3.0f, 3.0f, -8.0f}, 2.4f);
    m_sphereMaterials.emplace_back(Material::Type::Metal, 1.0f, 0.0f, glm::vec3{0.7f, 0.6f, 0.5f});
}

void RayTracerApp::createRenderScene(int index) {
    switch (index) {
    case 0:
        createScene1();
        break;
    case 1:
        createScene2();
        break;
    case 2:
        createScene3();
        break;
    default:
        throw std::invalid_argument("Invalid ray-tracing scene index");
    }

    m_activeSceneIndex = index;
    resetAccumulation();
}

void RayTracerApp::createScene1() {
    m_camera.transform.position = {0.0f, 0.0f, 12.0f};
    m_camera.transform.lookAt(glm::vec3{0.0f});

    createTracerScene({{{0.0f, 0.0f, 0.0f}, 1.5f}, {{4.0f, 0.0f, 0.0f}, 1.5f}, {{-4.0f, 0.0f, 0.0f}, 1.5f}},
                      {},
                      {},
                      {{Material::Type::Dielectric, 1.5f, 0.0f, {1.0f, 1.0f, 1.0f}},
                       {Material::Type::Metal, 1.0f, 0.0f, {0.7f, 0.6f, 0.5f}},
                       {Material::Type::Lambertian, 1.0f, 0.0f, {0.8f, 0.4f, 0.2f}}},
                      {});
}

void RayTracerApp::createScene2() {
    m_camera.transform.position = {12.48607f, 1.925702f, 1.317638f};
    m_camera.transform.lookAt({0.0f, 3.0f, 0.0f});

    createTracerScene(m_spheres, {}, {}, m_sphereMaterials, {});
}

void RayTracerApp::createScene3() {
    m_camera.transform.position = {12.48607f, 1.925702f, 1.317638f};
    m_camera.transform.lookAt({0.0f, 3.0f, 0.0f});

    auto const scale = glm::scale(glm::mat4{1.0f}, glm::vec3{0.6f});
    auto const rotation = glm::rotate(glm::mat4{1.0f}, glm::radians(90.0f), glm::vec3{0.0f, 1.0f, 0.0f});
    createTracerScene(m_spheres,
                      {m_lucy.get(), m_lucy.get(), m_lucy.get()},
                      {rotation * scale,
                       glm::translate(glm::mat4{1.0f}, glm::vec3{-4.0f, 0.0f, 2.0f}) * rotation * scale,
                       glm::translate(glm::mat4{1.0f}, glm::vec3{4.0f, 0.0f, -2.0f}) * rotation * scale},
                      m_sphereMaterials,
                      {{Material::Type::Dielectric, 1.5f, 0.0f, {1.0f, 1.0f, 1.0f}},
                       {Material::Type::Metal, 1.0f, 0.0f, {0.7f, 0.6f, 0.5f}},
                       {Material::Type::Lambertian, 1.0f, 0.0f, {0.8f, 0.4f, 0.2f}}});
}

void RayTracerApp::createTracerScene(std::vector<Sphere> const& spheres,
                                     std::vector<Model const*> const& models,
                                     std::vector<glm::mat4> const& transforms,
                                     std::vector<Material> const& sphereMaterials,
                                     std::vector<Material> const& modelMaterials) {
    auto scene = std::make_shared<Scene>(spheres, models, transforms, sphereMaterials, modelMaterials);
    m_cpuTracer->setScene(scene);
    m_gpuTracer->setScene(scene);
}

void RayTracerApp::drawScreenQuad() const {
    m_screenQuad->bind();
    gfx::RHI::draw(gfx::RHI::Primitive::Triangles, 0, 3);
    gfx::VertexArray::unbind();
}

void RayTracerApp::initUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    auto* window = static_cast<GLFWwindow*>(m_window.handle());
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        ImGui::DestroyContext();
        throw std::runtime_error("Unable to initialize ImGui GLFW backend");
    }
    if (!ImGui_ImplOpenGL3_Init("#version 410 core")) {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        throw std::runtime_error("Unable to initialize ImGui OpenGL backend");
    }
}

void RayTracerApp::renderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2{0.0f, 0.0f}, ImGuiCond_Once);
    auto const flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("Control Panel", nullptr, flags)) {
        ImGui::Text("renderer");
        ImGui::Separator();
        static char const* renderers[]{"GPU", "CPU"};
        if (ImGui::Combo("##renderer", &m_rendererIndex, renderers, IM_ARRAYSIZE(renderers))) {
            resetAccumulation();
        }
        ImGui::NewLine();
        ImGui::Text("switch scenes");
        ImGui::Separator();
        static char const* scenes[]{"scene 1", "scene 2", "scene 3"};
        ImGui::Combo("##1", &m_renderSceneIndex, scenes, IM_ARRAYSIZE(scenes));
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void RayTracerApp::resetAccumulation() noexcept {
    m_gpuTracer->reset();
    m_cpuTracer->reset();
}
