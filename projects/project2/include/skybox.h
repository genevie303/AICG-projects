#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include <graphics/texture/texture_cubemap.h>

class Skybox {
public:
    explicit Skybox(std::array<std::filesystem::path, 6> const& facePaths);

    glm::vec3 sample(glm::vec3 direction) const;

    gfx::TextureCubemap const& texture() const noexcept {
        return *m_texture;
    }

private:
    uint32_t m_width{0};
    uint32_t m_height{0};
    std::array<std::vector<uint8_t>, 6> m_faces;
    std::unique_ptr<gfx::TextureCubemap> m_texture;
};
