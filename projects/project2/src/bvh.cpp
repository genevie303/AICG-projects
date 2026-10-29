#include "bvh.h"

#include <algorithm>
#include <limits>
#include <numeric>

BVH::BVH(std::vector<Sphere> const& spheres,
         std::vector<Primitive> const& primitives,
         std::vector<Triangle> const& triangles,
         std::vector<Vertex> const& vertices)
    : m_primitiveBounds(primitives.size())
    , m_orderedPrimitiveIndices(primitives.size()) {
    for (size_t i = 0; i < primitives.size(); ++i) {
        auto const& primitive = primitives[i];
        auto& bounds = m_primitiveBounds[i];
        if (primitive.type == Primitive::Type::Sphere) {
            auto const& sphere = spheres[primitive.shapeIndex];
            bounds.expand(sphere.position - glm::vec3{sphere.radius});
            bounds.expand(sphere.position + glm::vec3{sphere.radius});
        }
        else {
            for (auto index : triangles[primitive.shapeIndex].v) {
                bounds.expand(vertices[index].position);
            }
        }
    }

    std::iota(m_orderedPrimitiveIndices.begin(), m_orderedPrimitiveIndices.end(), 0u);
    if (!primitives.empty()) {
        m_nodes.reserve(primitives.size() * 2 - 1);
        build(0, primitives.size(), 1);
    }
}

int BVH::build(size_t begin, size_t end, size_t depth) {
    m_height = std::max(m_height, depth);
    auto const nodeIndex = static_cast<int>(m_nodes.size());
    m_nodes.emplace_back();

    AABB bounds;
    AABB centroidBounds;
    for (size_t i = begin; i < end; ++i) {
        auto const& primitiveBounds = m_primitiveBounds[m_orderedPrimitiveIndices[i]];
        bounds.expand(primitiveBounds);
        centroidBounds.expand(primitiveBounds.centroid());
    }

    auto const count = end - begin;
    auto const axis = centroidBounds.maximumExtentAxis();
    auto const min = centroidBounds.min[axis];
    auto const max = centroidBounds.max[axis];
    // Match the reference's one-primitive leaves and degenerate-centroid leaves.
    // Cap depth so the shader's fixed 64-entry traversal stack cannot overflow.
    if (count == 1 || min == max || depth >= 64) {
        m_nodes[nodeIndex] = {bounds, static_cast<int>(begin), -static_cast<int>(count)};
        return nodeIndex;
    }

    constexpr int bucketCount = 12;
    struct Bucket {
        size_t count{0};
        AABB bounds;
    };
    Bucket buckets[bucketCount];
    auto bucketIndex = [&](uint32_t index) {
        auto const centroid = m_primitiveBounds[index].centroid()[axis];
        return std::clamp(static_cast<int>(bucketCount * (centroid - min) / (max - min)), 0, bucketCount - 1);
    };
    for (size_t i = begin; i < end; ++i) {
        auto const index = m_orderedPrimitiveIndices[i];
        auto& bucket = buckets[bucketIndex(index)];
        ++bucket.count;
        bucket.bounds.expand(m_primitiveBounds[index]);
    }

    float minCost = std::numeric_limits<float>::infinity();
    int splitBucket = 0;
    for (int split = 0; split < bucketCount - 1; ++split) {
        AABB leftBounds;
        AABB rightBounds;
        size_t leftCount = 0;
        size_t rightCount = 0;
        for (int i = 0; i < bucketCount; ++i) {
            if (i <= split) {
                leftCount += buckets[i].count;
                leftBounds.expand(buckets[i].bounds);
            }
            else {
                rightCount += buckets[i].count;
                rightBounds.expand(buckets[i].bounds);
            }
        }
        if (leftCount == 0 || rightCount == 0) {
            continue;
        }

        // The reference's centroid-area denominator is common to all splits.
        // Comparing numerators also handles a zero-area centroid bound.
        auto const cost = leftCount * leftBounds.surfaceArea() + rightCount * rightBounds.surfaceArea();
        if (cost < minCost) {
            minCost = cost;
            splitBucket = split;
        }
    }

    auto const split = std::partition(m_orderedPrimitiveIndices.begin() + begin,
                                      m_orderedPrimitiveIndices.begin() + end,
                                      [&](uint32_t index) { return bucketIndex(index) <= splitBucket; });
    auto middle = static_cast<size_t>(split - m_orderedPrimitiveIndices.begin());
    if (middle == begin || middle == end) {
        middle = begin + count / 2;
        std::nth_element(m_orderedPrimitiveIndices.begin() + begin,
                         m_orderedPrimitiveIndices.begin() + middle,
                         m_orderedPrimitiveIndices.begin() + end,
                         [&](uint32_t lhs, uint32_t rhs) {
                             return m_primitiveBounds[lhs].centroid()[axis] < m_primitiveBounds[rhs].centroid()[axis];
                         });
    }

    auto const left = build(begin, middle, depth + 1);
    auto const right = build(middle, end, depth + 1);
    m_nodes[nodeIndex] = {bounds, left, right};

    return nodeIndex;
}
