#pragma once

#include <filesystem>
#include <vector>

#include "triangle.h"
#include "vertex.h"

class Model {
public:
    explicit Model(std::filesystem::path const& path);

    std::vector<Vertex> const& vertices() const noexcept {
        return m_vertices;
    }

    std::vector<Triangle> const& triangles() const noexcept {
        return m_triangles;
    }

private:
    std::vector<Vertex> m_vertices;
    std::vector<Triangle> m_triangles;
};
