#include "scene.h"

#include <cstdint>
#include <iostream>
#include <utility>

Scene::Scene(std::vector<Sphere> const& spheres,
             std::vector<Model const*> const& models,
             std::vector<glm::mat4> const& transforms,
             std::vector<Material> const& sphereMaterials,
             std::vector<Material> const& modelMaterials)
    : m_spheres{spheres} {
    size_t totalVertices = 0;
    size_t totalTriangles = 0;
    for (auto const* model : models) {
        totalVertices += model->vertices().size();
        totalTriangles += model->triangles().size();
    }

    m_vertices.reserve(totalVertices);
    m_triangles.reserve(totalTriangles);
    m_primitives.reserve(spheres.size() + totalTriangles);
    m_materials.reserve(sphereMaterials.size() + modelMaterials.size());
    m_materials.insert(m_materials.end(), sphereMaterials.begin(), sphereMaterials.end());
    m_materials.insert(m_materials.end(), modelMaterials.begin(), modelMaterials.end());

    for (size_t i = 0; i < spheres.size(); ++i) {
        m_primitives.push_back({Primitive::Type::Sphere, static_cast<uint32_t>(i), static_cast<uint32_t>(i)});
    }
    for (size_t i = 0; i < models.size(); ++i) {
        auto const vertexOffset = static_cast<uint32_t>(m_vertices.size());
        auto const normalTransform = glm::mat3(glm::transpose(glm::inverse(transforms[i])));
        for (auto const& vertex : models[i]->vertices()) {
            m_vertices.push_back(
                {glm::vec3(transforms[i] * glm::vec4(vertex.position, 1.0f)),
                 normalTransform * vertex.normal,
                 vertex.texCoord});
        }
        for (auto const& triangle : models[i]->triangles()) {
            m_primitives.push_back(
                {Primitive::Type::Triangle,
                 static_cast<uint32_t>(m_triangles.size()),
                 static_cast<uint32_t>(sphereMaterials.size() + i)});
            m_triangles.push_back(
                {{triangle.v[0] + vertexOffset, triangle.v[1] + vertexOffset, triangle.v[2] + vertexOffset}});
        }
    }

    BVH bvh{m_spheres, m_primitives, m_triangles, m_vertices};
    std::vector<Primitive> orderedPrimitives;
    orderedPrimitives.reserve(m_primitives.size());
    for (auto index : bvh.orderedPrimitiveIndices()) {
        orderedPrimitives.push_back(m_primitives[index]);
    }
    m_primitives = std::move(orderedPrimitives);

    std::cout << "Scene Statistics\n"
              << "+ Spheres: " << spheres.size() << '\n'
              << "+ Models:  " << models.size() << '\n'
              << "  + vertices:  " << totalVertices << '\n'
              << "  + triangles: " << totalTriangles << '\n'
              << "+ BVH:\n"
              << "  + nodes: " << bvh.nodes().size() << '\n'
              << "  + depth: " << bvh.height() << std::endl;
    m_nodes = std::move(bvh).takeNodes();
}
