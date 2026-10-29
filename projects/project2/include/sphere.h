#pragma once

#include <glm/glm.hpp>

struct Sphere {
public:
    glm::vec3 position;
    float radius;

public:
    Sphere() = default;

    Sphere(glm::vec3 const& position, float radius)
        : position(position)
        , radius(radius) {}

    static constexpr int getTexDataComponent() noexcept {
        return 4;
    }
};
