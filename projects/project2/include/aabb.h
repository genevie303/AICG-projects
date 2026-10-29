#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

#include <glm/glm.hpp>

#include "ray.h"

struct AABB {
    glm::vec3 min{std::numeric_limits<float>::max()};
    glm::vec3 max{std::numeric_limits<float>::lowest()};

    void expand(glm::vec3 const& point) noexcept {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    void expand(AABB const& box) noexcept {
        if (box.min.x > box.max.x || box.min.y > box.max.y || box.min.z > box.max.z) {
            return;
        }

        expand(box.min);
        expand(box.max);
    }

    glm::vec3 centroid() const noexcept {
        return 0.5f * (min + max);
    }

    int maximumExtentAxis() const noexcept {
        auto const extent = max - min;
        if (extent.x > extent.y && extent.x > extent.z) {
            return 0;
        }
        return extent.y > extent.z ? 1 : 2;
    }

    float surfaceArea() const noexcept {
        auto const extent = max - min;
        if (extent.x < 0.0f || extent.y < 0.0f || extent.z < 0.0f) {
            return 0.0f;
        }
        return 2.0f * (extent.x * extent.y + extent.x * extent.z + extent.y * extent.z);
    }

    bool intersects(Ray const& ray) const noexcept {
        float near = 0.0f;
        float far = ray.tMax;
        for (int axis = 0; axis < 3; ++axis) {
            if (std::abs(ray.direction[axis]) < 1e-8f) {
                if (ray.origin[axis] < min[axis] || ray.origin[axis] > max[axis]) {
                    return false;
                }
                continue;
            }
            auto first = (min[axis] - ray.origin[axis]) / ray.direction[axis];
            auto second = (max[axis] - ray.origin[axis]) / ray.direction[axis];
            if (first > second) {
                std::swap(first, second);
            }
            near = std::max(near, first);
            far = std::min(far, second);
            if (near > far) {
                return false;
            }
        }
        return true;
    }
};
