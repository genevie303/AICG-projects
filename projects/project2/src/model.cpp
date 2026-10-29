#include "model.h"

#include <stdexcept>
#include <unordered_map>

#include <tiny_obj_loader.h>

namespace {
struct VertexIndex {
    int position;
    int normal;
    int texCoord;

    bool operator==(VertexIndex const&) const = default;
};

struct VertexIndexHash {
    size_t operator()(VertexIndex const& index) const noexcept {
        auto hash = std::hash<int>{}(index.position);
        hash ^= std::hash<int>{}(index.normal) + 0x9e3779b9u + (hash << 6) + (hash >> 2);
        hash ^= std::hash<int>{}(index.texCoord) + 0x9e3779b9u + (hash << 6) + (hash >> 2);
        return hash;
    }
};
} // namespace

Model::Model(std::filesystem::path const& path) {
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig config;
    config.triangulate = true;
    config.mtl_search_path = path.parent_path().string();
    if (!reader.ParseFromFile(path.string(), config)) {
        throw std::runtime_error("Unable to load model: " + path.string() + "\n" + reader.Error());
    }

    auto const& attributes = reader.GetAttrib();
    std::unordered_map<VertexIndex, uint32_t, VertexIndexHash> indices;
    indices.reserve(attributes.vertices.size() / 3);

    for (auto const& shape : reader.GetShapes()) {
        size_t offset = 0;
        for (auto count : shape.mesh.num_face_vertices) {
            if (count != 3) {
                throw std::runtime_error("Model contains a non-triangulated face: " + path.string());
            }

            Triangle triangle;
            for (size_t i = 0; i < 3; ++i) {
                auto const index = shape.mesh.indices.at(offset + i);
                VertexIndex const key{index.vertex_index, index.normal_index, index.texcoord_index};
                auto const [it, inserted] = indices.try_emplace(key, static_cast<uint32_t>(m_vertices.size()));
                if (inserted) {
                    Vertex vertex;
                    for (size_t axis = 0; axis < 3; ++axis) {
                        vertex.position[axis] = attributes.vertices.at(3 * index.vertex_index + axis);
                        if (index.normal_index >= 0) {
                            vertex.normal[axis] = attributes.normals.at(3 * index.normal_index + axis);
                        }
                    }
                    if (index.texcoord_index >= 0) {
                        vertex.texCoord = {
                            attributes.texcoords.at(2 * index.texcoord_index),
                            attributes.texcoords.at(2 * index.texcoord_index + 1)
                        };
                    }
                    m_vertices.push_back(vertex);
                }
                triangle.v[i] = it->second;
            }

            m_triangles.push_back(triangle);
            offset += 3;
        }
    }

    if (m_triangles.empty()) {
        throw std::runtime_error("Model contains no triangles: " + path.string());
    }

    // Accumulate geometric normals only for vertices without OBJ normals.
    std::vector<glm::vec3> generatedNormals(m_vertices.size(), glm::vec3{0.0f});
    for (auto const& triangle : m_triangles) {
        auto const normal = glm::cross(m_vertices[triangle.v[1]].position - m_vertices[triangle.v[0]].position,
                                       m_vertices[triangle.v[2]].position - m_vertices[triangle.v[0]].position);
        for (auto index : triangle.v) {
            generatedNormals[index] += normal;
        }
    }
    for (size_t i = 0; i < m_vertices.size(); ++i) {
        if (glm::dot(m_vertices[i].normal, m_vertices[i].normal) == 0.0f &&
            glm::dot(generatedNormals[i], generatedNormals[i]) > 0.0f) {
            m_vertices[i].normal = glm::normalize(generatedNormals[i]);
        }
    }
}
