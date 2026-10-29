#pragma once

#include <limits>

#include <glm/glm.hpp>

struct Ray {
    glm::vec3 origin{0.0f};
    glm::vec3 direction{0.0f, 0.0f, -1.0f};
    float tMax{std::numeric_limits<float>::max()};

    glm::vec3 at(float t) const noexcept {
        return origin + t * direction;
    }
};
