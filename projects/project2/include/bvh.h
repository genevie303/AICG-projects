#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "aabb.h"
#include "primitive.h"
#include "sphere.h"
#include "triangle.h"
#include "vertex.h"

struct BVHNode {
    AABB bounds;
    // Interior: first/second are left/right child node indices (second >= 0).
    // Leaf: first is the first primitive index; second is the negative primitive count.
    int first{0};
    int second{0};

    bool isLeaf() const noexcept {
        return second < 0;
    }
};

class BVH {
public:
    using Node = BVHNode;

    BVH(std::vector<Sphere> const& spheres,
        std::vector<Primitive> const& primitives,
        std::vector<Triangle> const& triangles,
        std::vector<Vertex> const& vertices);

    std::vector<Node> const& nodes() const noexcept {
        return m_nodes;
    }

    std::vector<Node> takeNodes() && noexcept {
        return std::move(m_nodes);
    }

    std::vector<uint32_t> const& orderedPrimitiveIndices() const noexcept {
        return m_orderedPrimitiveIndices;
    }

    size_t height() const noexcept {
        return m_height;
    }

private:
    std::vector<AABB> m_primitiveBounds;
    std::vector<Node> m_nodes;
    std::vector<uint32_t> m_orderedPrimitiveIndices;
    size_t m_height{0};

    int build(size_t begin, size_t end, size_t depth);
};
