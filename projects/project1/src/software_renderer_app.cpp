#include "software_renderer_app.h"

#include <cstdint>
#include <format>
#include <limits>
#include <stdexcept>

#include <common/file_utils.h>
#include <common/event/event_dispatcher.h>
#include <common/event/keyboard_event.h>
#include <common/event/window_event.h>
#include <glm/ext.hpp>
#include <graphics/rhi.h>
#include <graphics/shader_module.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

constexpr char const* kModelPath = "models/DamagedHelmet.gltf";
constexpr char const* kTransformPath = "shaders/transform.vert";
constexpr char const* kRasterVertexPath = "shaders/raster_input.vert";
constexpr char const* kRasterGeometryPath = "shaders/raster_bounds.geom";
constexpr char const* kShadingPath = "shaders/raster_shading.frag";
constexpr char const* kDisplayVertexPath = "shaders/display.vert";
constexpr char const* kDisplayFragmentPath = "shaders/display.frag";
constexpr float kModelRotateSpeed = 0.5f;

SoftwareRendererApp::SoftwareRendererApp(Config const& config)
    : Application(config) {
    // scene
    m_scene = std::make_unique<RasterScene>(getAssetPath(kModelPath));

    // camera
    m_camera.aspect = m_window.aspectRatio();
    m_camera.transform.position = {0.0f, 0.0f, 3.6f};

    // light
    m_directionalLight.transform.rotation =
        glm::quatLookAt(glm::normalize(glm::vec3(0.5f, -0.5f, -0.70710678f)), Transform::getDefaultUp());
    m_directionalLight.intensity = 10.0f;

    // cpu renderer
    m_cpuRenderer = std::make_unique<CpuRenderer>(m_window.framebufferWidth(), m_window.framebufferHeight());

    // gpu renderer
    m_gpuRenderer = std::make_unique<GpuRenderer>(m_window.framebufferWidth(),
                                                  m_window.framebufferHeight(),
                                                  getAssetPath(kTransformPath),
                                                  getAssetPath(kRasterVertexPath),
                                                  getAssetPath(kRasterGeometryPath),
                                                  getAssetPath(kShadingPath));

    // display program
    std::vector<gfx::ShaderModule> shaderModules;
    shaderModules.emplace_back(readTextFile(getAssetPath(kDisplayVertexPath)), gfx::ShaderModule::Stage::Vertex);
    shaderModules.emplace_back(readTextFile(getAssetPath(kDisplayFragmentPath)), gfx::ShaderModule::Stage::Fragment);
    m_displayProgram = std::make_unique<gfx::GLProgram>(shaderModules);

    // ui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    if (!ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(m_window.handle()), true)) {
        ImGui::DestroyContext();
        throw std::runtime_error("ImGui GLFW initialization failed");
    }
    if (!ImGui_ImplOpenGL3_Init("#version 410")) {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        throw std::runtime_error("ImGui OpenGL initialization failed");
    }
    ImGui::GetIO().IniFilename = nullptr;

    // reset clock
    resetClock();
}

SoftwareRendererApp::~SoftwareRendererApp() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void SoftwareRendererApp::validateDrawItem(DrawItem const& drawItem) {
    auto const& indices = drawItem.mesh->indices();
    if (indices.size() % 3 != 0) {
        throw std::invalid_argument("Mesh indices must describe complete triangles");
    }
    if (indices.size() > static_cast<size_t>(std::numeric_limits<int32_t>::max())) {
        throw std::runtime_error("Too many indices for the GPU vertex pass");
    }
    for (uint32_t index : indices) {
        if (index >= drawItem.mesh->vertices().size()) {
            throw std::invalid_argument("Mesh index references a missing vertex");
        }
    }
}

FrameState SoftwareRendererApp::frameForDrawItem(FrameState frame, DrawItem const& drawItem) {
    frame.model = frame.model * drawItem.meshToScene;
    return frame;
}

void SoftwareRendererApp::handleEvent(Event& event) {
    EventDispatcher dispatcher{event};
    dispatcher.dispatch<WindowFramebufferResizeEvent>([this](WindowFramebufferResizeEvent& resizeEvent) {
        if (resizeEvent.getWidth() == 0 || resizeEvent.getHeight() == 0) {
            return false;
        }
        glm::uvec2 const extent{resizeEvent.getWidth(), resizeEvent.getHeight()};
        m_cpuRenderer->resize(extent);
        m_gpuRenderer->resize(extent);
        m_camera.aspect = m_window.aspectRatio();

        return false;
    });

    dispatcher.dispatch<KeyPressEvent>([this](KeyPressEvent& key) {
        if (key.getKeyCode() == KeyCode::Escape) {
            close();
            return true;
        }
        return false;
    });
}

void SoftwareRendererApp::renderFrame() {
    auto const width = m_window.framebufferWidth();
    auto const height = m_window.framebufferHeight();
    if (width == 0 || height == 0) {
        return;
    }

    float const deltaTime = getDeltaTime();
    float const fps = deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;
    m_window.setTitle(std::format("Software Renderer | {:.1f} FPS", fps));

    // render the scene
    FrameState sceneFrame{
        .model = glm::rotate(glm::mat4(1.0f), getTotalTime() * kModelRotateSpeed, glm::vec3(0.0f, 1.0f, 0.0f)),
        .view = m_camera.getViewMatrix(),
        .projection = m_camera.getProjectionMatrix(),
        .cameraPosition = m_camera.transform.position,
        .directionalLight = m_directionalLight,
        .extent = {width, height},
    };

    Renderer& renderer = m_renderBackend == RenderBackend::Cpu ? static_cast<Renderer&>(*m_cpuRenderer)
                                                               : static_cast<Renderer&>(*m_gpuRenderer);
    renderer.clear();
    for (DrawItem const& drawItem : m_scene->drawItems()) {
        validateDrawItem(drawItem);
        renderer.render(drawItem, frameForDrawItem(sceneFrame, drawItem));
    }

    // display the render result on the screen
    gfx::RHI::enableDepthTest(false);
    gfx::RHI::setViewport({0, 0, width, height});

    m_displayProgram->use();

    auto const hdrTextureUnit = m_displayProgram->getTextureBinding("uHdrColor");
    auto const& output = renderer.outputTexture();
    output.bind(hdrTextureUnit);
    m_displaySampler.bind(hdrTextureUnit);

    m_fullscreenVao.bind();
    gfx::RHI::draw(gfx::RHI::Primitive::Triangles, 0, 3);
    gfx::VertexArray::unbind();

    gfx::Sampler::unbind(hdrTextureUnit);
    output.unbind();

    // render UI elements
    renderUi();
}

void SoftwareRendererApp::renderUi() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuiWindowFlags const flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("Rasterizer", nullptr, flags)) {
        int mode = static_cast<int>(m_renderBackend);
        ImGui::RadioButton("CPU", &mode, static_cast<int>(RenderBackend::Cpu));
        ImGui::SameLine();
        ImGui::RadioButton("GPU fragment", &mode, static_cast<int>(RenderBackend::Gpu));
        m_renderBackend = static_cast<RenderBackend>(mode);
        ImGui::Separator();
        ImGui::ColorEdit3("light color", &m_directionalLight.color.x);
        ImGui::SliderFloat("light intensity", &m_directionalLight.intensity, 0.0f, 20.0f);
    }
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
