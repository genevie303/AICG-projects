#include "skybox.h"

#include <algorithm>
#include <memory>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Skybox::Skybox(std::array<std::filesystem::path, 6> const& facePaths) {
    for (size_t i = 0; i < facePaths.size(); ++i) {
        int loadedWidth{};
        int loadedHeight{};
        int channels{};
        auto const& path = facePaths[i];
        std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> pixels{
            stbi_load(path.string().c_str(), &loadedWidth, &loadedHeight, &channels, STBI_rgb), stbi_image_free
        };
        if (!pixels) {
            throw std::runtime_error("Unable to load skybox texture: " + path.string());
        }
        if (i != 0 && (loadedWidth != m_width || loadedHeight != m_height)) {
            throw std::runtime_error("Skybox faces must use the same dimensions");
        }
        m_width = static_cast<uint32_t>(loadedWidth);
        m_height = static_cast<uint32_t>(loadedHeight);
        m_faces[i].assign(pixels.get(), pixels.get() + static_cast<size_t>(loadedWidth) * loadedHeight * 3);
    }

    std::array<void const*, 6> pixels{};
    for (size_t i = 0; i < pixels.size(); ++i) {
        pixels[i] = m_faces[i].data();
    }
    m_texture = std::make_unique<gfx::TextureCubemap>(
        gfx::TextureCubemap::Specification{
            .width = m_width,
            .height = m_height,
            .format = gfx::Texture::Format::RGB8,
            .pixelFormat = gfx::Texture::ExternalFormat::RGB,
            .pixelType = gfx::Texture::PixelType::UByte
        },
        pixels);
}

glm::vec3 Skybox::sample(glm::vec3 direction) const {
    auto const absolute = glm::abs(direction);
    int face;
    float u;
    float v;
    if (absolute.x >= absolute.y && absolute.x >= absolute.z) {
        face = direction.x >= 0.0f ? 0 : 1;
        u = (direction.x >= 0.0f ? -direction.z : direction.z) / absolute.x;
        v = -direction.y / absolute.x;
    }
    else if (absolute.y >= absolute.z) {
        face = direction.y >= 0.0f ? 2 : 3;
        u = direction.x / absolute.y;
        v = (direction.y >= 0.0f ? direction.z : -direction.z) / absolute.y;
    }
    else {
        face = direction.z >= 0.0f ? 4 : 5;
        u = (direction.z >= 0.0f ? direction.x : -direction.x) / absolute.z;
        v = -direction.y / absolute.z;
    }

    auto const x = std::clamp((u + 1.0f) * 0.5f * m_width - 0.5f, 0.0f, static_cast<float>(m_width - 1));
    auto const y = std::clamp((v + 1.0f) * 0.5f * m_height - 0.5f, 0.0f, static_cast<float>(m_height - 1));
    auto const x0 = static_cast<uint32_t>(x);
    auto const y0 = static_cast<uint32_t>(y);
    auto const x1 = std::min(x0 + 1, m_width - 1);
    auto const y1 = std::min(y0 + 1, m_height - 1);
    auto const& data = m_faces[face];
    auto samplePixel = [&](uint32_t sx, uint32_t sy) {
        auto const offset = (static_cast<size_t>(sy) * m_width + sx) * 3;
        return glm::vec3{data[offset], data[offset + 1], data[offset + 2]} / 255.0f;
    };
    auto const top = glm::mix(samplePixel(x0, y0), samplePixel(x1, y0), x - x0);
    auto const bottom = glm::mix(samplePixel(x0, y1), samplePixel(x1, y1), x - x0);

    return glm::mix(top, bottom, y - y0);
}
