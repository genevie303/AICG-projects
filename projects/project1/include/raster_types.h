#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

#include <glm/glm.hpp>

#include <scene/light/light.h>
#include <graphics/sampler.h>
#include <graphics/texture/texture2d.h>

#include "cpu_sampler.h"
#include "cpu_texture2d.h"
#include "mesh.h"

struct UnifiedTexture2D {
    // Texture storage and sampling state for the two rasterizers.
    std::shared_ptr<CpuTexture2D> cpuTexture;
    std::shared_ptr<CpuSampler> cpuSampler;
    std::shared_ptr<gfx::Texture2D> gpuTexture;
    std::shared_ptr<gfx::Sampler> gpuSampler;
};

struct PbrMaterial {
    // glTF baseColorFactor, multiplied with the base-color texture.
    glm::vec4 baseColorFactor{1.0f};
    // glTF emissiveFactor, added after direct-light evaluation.
    glm::vec3 emissiveFactor{0.0f};
    // Scalar fallback/multiplier for the metallic texture's B channel.
    float metallicFactor = 1.0f;
    // Scalar fallback/multiplier for the roughness texture's G channel.
    float roughnessFactor = 1.0f;
    // Weight applied to the occlusion texture's R channel.
    float occlusionStrength = 1.0f;

    enum TextureSlot : int { BaseColor = 0, MetallicRoughness, Normal, Occlusion, Emissive };

    // Avoid sampling an unbound texture when a glTF slot is absent.
    std::array<bool, 5> hasTexture{};
    // Identical texture set exposed to both rasterizers.
    std::array<UnifiedTexture2D, 5> textures;
};

struct DrawItem {
    Mesh const* mesh = nullptr;
    PbrMaterial const* material = nullptr;
    // Mesh-local to scene-space transform accumulated through the node hierarchy.
    glm::mat4 meshToScene{1.0f};
};

// Shared vertex-stage layout, matching the GPU's interleaved transform feedback output.
struct ClipVertex {
    glm::vec4 clip{0.0f};
    glm::vec3 worldPosition{0.0f};
    glm::vec3 normal{0.0f, 1.0f, 0.0f};
    glm::vec2 uv{0.0f};
};

static_assert(sizeof(ClipVertex) == 48);
static_assert(offsetof(ClipVertex, clip) == 0);
static_assert(offsetof(ClipVertex, worldPosition) == 16);
static_assert(offsetof(ClipVertex, normal) == 28);
static_assert(offsetof(ClipVertex, uv) == 40);

struct FrameState {
    // Rendering inputs shared by both rasterizers; model is specific to the current draw item.
    // Mesh-to-world transform for the draw item currently being rendered.
    glm::mat4 model{1.0f};
    // Camera world-to-view transform.
    glm::mat4 view{1.0f};
    // Perspective projection used to create screen-space triangles.
    glm::mat4 projection{1.0f};
    // Needed to construct the PBR view vector.
    glm::vec3 cameraPosition{0.0f, 0.0f, 3.6f};
    // Keeps transform, color, and intensity as one component.
    DirectionalLight directionalLight;
    // Raster target size in physical framebuffer pixels.
    glm::uvec2 extent{1280, 720};
};
