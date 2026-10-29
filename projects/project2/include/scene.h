#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "bvh.h"
#include "material.h"
#include "model.h"

class Scene {
public:
    Scene(std::vector<Sphere> const& spheres,
          std::vector<Model const*> const& models,
          std::vector<glm::mat4> const& transforms,
          std::vector<Material> const& sphereMaterials,
          std::vector<Material> const& modelMaterials);

    std::vector<Sphere> const& spheres() const noexcept {
        return m_spheres;
    }

    std::vector<Primitive> const& primitives() const noexcept {
        return m_primitives;
    }

    std::vector<Triangle> const& triangles() const noexcept {
        return m_triangles;
    }

    std::vector<Vertex> const& vertices() const noexcept {
        return m_vertices;
    }

    std::vector<Material> const& materials() const noexcept {
        return m_materials;
    }

    std::vector<BVHNode> const& nodes() const noexcept {
        return m_nodes;
    }

private:
    std::vector<Sphere> m_spheres;
    std::vector<Primitive> m_primitives;
    std::vector<Triangle> m_triangles;
    std::vector<Vertex> m_vertices;
    std::vector<Material> m_materials;
    std::vector<BVHNode> m_nodes;
};
