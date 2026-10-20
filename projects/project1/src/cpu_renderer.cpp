#define GLM_ENABLE_EXPERIMENTAL
#include "cpu_renderer.h"

#include <array>
#include <cmath>
#include <glm/gtx/exterior_product.hpp>

#include "pbr_shading.h"

// CPU geometry-stage triangle with the attributes needed for software rasterization.
struct ScreenVertex {
    // xy is pixel-space position, z is normalized depth, and w is 1 / clip-space w.
    glm::vec4 screenAndInvW{0.0f};
    // World-space position and unnormalized normal to interpolate for lighting.
    glm::vec3 worldPosition{0.0f};
    glm::vec3 normal{0.0f, 1.0f, 0.0f};
    glm::vec2 uv{0.0f};
};

struct ScreenTriangle {
    // Clockwise/back-facing triangles are removed before this stage.
    ScreenVertex vertices[3];
    // UV-derived world-space T basis for normal mapping.
    glm::vec3 tangent{1.0f, 0.0f, 0.0f};
    // UV-derived world-space B basis for normal mapping.
    glm::vec3 bitangent{0.0f, 1.0f, 0.0f};
};

// A clipping plane can cross at most two edges of a convex polygon. It adds two
// intersection vertices but removes at least one old vertex, so the count grows
// by at most one per plane: 3 triangle vertices + 6 clip planes = 9.
constexpr size_t kMaxClippedPolygonVertices = 3 + 6;

// Transform each unique mesh vertex once; indexed triangles reuse these results later.
static std::vector<ClipVertex> transformVertices(Mesh const& mesh, FrameState const& frame) {
    std::vector<ClipVertex> vertices;
    vertices.reserve(mesh.vertices().size());

    // The app has already combined the scene and node transforms in frame.model.
    // Use its inverse transpose for normals under non-uniform scaling.
    glm::mat3 const normalTransform = glm::transpose(glm::inverse(glm::mat3(frame.model)));
    glm::mat4 const modelViewProjection = frame.projection * frame.view * frame.model;

    for (auto const& vertex : mesh.vertices()) {
        glm::vec4 const position(vertex.position, 1.0f);
        glm::vec4 const clip = modelViewProjection * position;
        glm::vec3 const worldPosition = glm::vec3(frame.model * position);
        glm::vec3 const worldNormal = glm::normalize(normalTransform * vertex.normal);

        // UVs belong to the mesh surface, so geometric transforms leave them unchanged.
        vertices.push_back({clip, worldPosition, worldNormal, vertex.uv});
    }

    return vertices;
}

// Clip a polygon against one plane in homogeneous clip space. For
// plane = (a, b, c, d), the boundary is a*x + b*y + c*z + d*w = 0.
// Keep the half-space where dot(plane, vertex.clip) >= 0; the other side is outside.
// polygon and count are updated in place; scratch storage keeps input edges intact.
static void
clipAgainst(std::array<ClipVertex, kMaxClippedPolygonVertices>& polygon, size_t& count, glm::vec4 const& plane) {
    /**************************************** Todo Begin *****************************************/
    std::array<ClipVertex, kMaxClippedPolygonVertices> clipped;
    size_t outCount = 0;

    for (size_t i = 0; i < count; ++i) {
        ClipVertex const& curr = polygon[i];
        ClipVertex const& next = polygon[(i + 1) % count];

        float dCurr = glm::dot(plane, curr.clip);
        float dNext = glm::dot(plane, next.clip);

        if (dCurr >= 0.0f) {
            clipped[outCount++] = curr;
        }

        if ((dCurr >= 0.0f) != (dNext >= 0.0f)) {
            float t = dCurr / (dCurr - dNext);
            ClipVertex intersect;
            intersect.clip          = glm::mix(curr.clip, next.clip, t);
            intersect.worldPosition = glm::mix(curr.worldPosition, next.worldPosition, t);
            intersect.normal        = glm::mix(curr.normal, next.normal, t);
            intersect.uv            = glm::mix(curr.uv, next.uv, t);
            clipped[outCount++] = intersect;
        }
    }

    for (size_t i = 0; i < outCount; ++i) polygon[i] = clipped[i];
    count = outCount;
    /***************************************** Todo End ******************************************/
}

// Assemble indexed clip-space vertices into clipped, front-facing screen triangles.
static std::vector<ScreenTriangle>
buildScreenTriangles(std::vector<ClipVertex> const& vertices, std::vector<uint32_t> const& indices, glm::uvec2 extent) {
    std::vector<ScreenTriangle> result;
    result.reserve(indices.size() / 3);

    // Each consecutive group of three indices names one source triangle.
    for (size_t index = 0; index < indices.size(); index += 3) {
        // Fixed storage covers the worst-case clipped polygon. Only the first
        // count entries are valid; initially they are the source triangle.
        std::array<ClipVertex, kMaxClippedPolygonVertices> polygon;
        size_t count = 3;
        for (int corner = 0; corner < 3; ++corner) {
            auto const vertexIndex = indices[index + corner];
            polygon[corner] = vertices[vertexIndex];
        }

        // Perform Sutherland-Hodgeman algorithm to clip the triangle with 6 planes

        // OpenGL's homogeneous clip volume is -w <= x, y, z <= w.
        // A nonnegative result means the vertex is inside the corresponding plane.
        // Clipping may discard the triangle or add intersection vertices.
        clipAgainst(polygon, count, {1.0f, 0.0f, 0.0f, 1.0f});  // left: x + w >= 0
        clipAgainst(polygon, count, {-1.0f, 0.0f, 0.0f, 1.0f}); // right: w - x >= 0
        clipAgainst(polygon, count, {0.0f, 1.0f, 0.0f, 1.0f});  // bottom: y + w >= 0
        clipAgainst(polygon, count, {0.0f, -1.0f, 0.0f, 1.0f}); // top: w - y >= 0
        clipAgainst(polygon, count, {0.0f, 0.0f, 1.0f, 1.0f});  // near: z + w >= 0
        clipAgainst(polygon, count, {0.0f, 0.0f, -1.0f, 1.0f}); // far: w - z >= 0

        // The clipped polygon is convex. Fan it out as (0, 1, 2), (0, 2, 3), ...;
        // fewer than three surviving vertices naturally produce no triangles.
        for (size_t triangle = 1; triangle + 1 < count; ++triangle) {
            ClipVertex const clipped[] = {polygon[0], polygon[triangle], polygon[triangle + 1]};
            ScreenTriangle output{};

            /************************************ Todo Begin *************************************/
            // Convert each clip-space vertex to a ScreenVertex
            for (int i = 0; i < 3; ++i) {
                float invW = 1.0f / clipped[i].clip.w;
                glm::vec3 ndc = glm::vec3(clipped[i].clip) * invW;

                output.vertices[i].screenAndInvW = glm::vec4(
                    (ndc.x * 0.5f + 0.5f) * float(extent.x),
                    (ndc.y * 0.5f + 0.5f) * float(extent.y),
                    ndc.z * 0.5f + 0.5f,
                    invW
                );
                output.vertices[i].worldPosition = clipped[i].worldPosition;
                output.vertices[i].normal        = clipped[i].normal;
                output.vertices[i].uv            = clipped[i].uv;
            }

            // Compute tangent and bitangent from position + UV edge vectors
            glm::vec3 const dp1 = output.vertices[1].worldPosition - output.vertices[0].worldPosition;
            glm::vec3 const dp2 = output.vertices[2].worldPosition - output.vertices[0].worldPosition;
            glm::vec2 const duv1 = output.vertices[1].uv - output.vertices[0].uv;
            glm::vec2 const duv2 = output.vertices[2].uv - output.vertices[0].uv;
            float det = duv1.x * duv2.y - duv1.y * duv2.x;
            if (std::abs(det) > 1e-6f) {
                float invDet = 1.0f / det;
                output.tangent   = glm::normalize((dp1 * duv2.y - dp2 * duv1.y) * invDet);
                output.bitangent = glm::normalize((dp2 * duv1.x - dp1 * duv2.x) * invDet);
            } else {
                // Degenerate UVs: build orthonormal basis from normal
                glm::vec3 n = glm::normalize(output.vertices[0].normal);
                glm::vec3 up = (std::abs(n.y) < 0.99f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
                output.tangent   = glm::normalize(glm::cross(up, n));
                output.bitangent = glm::cross(n, output.tangent);
            }

            result.push_back(output);
            /************************************* Todo End **************************************/
        }
    }

    return result;
}

static void rasterizeTriangles(std::vector<ScreenTriangle> const& triangles,
                               PbrMaterial const& material,
                               FrameState const& frame,
                               glm::uvec2 extent,
                               std::vector<glm::vec4>& colorBuffer,
                               std::vector<float>& depthBuffer) {
    for (ScreenTriangle const& triangle : triangles) {
        glm::vec2 const a(triangle.vertices[0].screenAndInvW);
        glm::vec2 const b(triangle.vertices[1].screenAndInvW);
        glm::vec2 const c(triangle.vertices[2].screenAndInvW);
        float const area = glm::cross(b - a, c - a);
        if (area <= 0.0f) {
            continue;
        }

        // Restrict coverage tests to the triangle's screen-space bounding box.
        glm::vec2 const minPoint = glm::min(a, glm::min(b, c));
        glm::vec2 const maxPoint = glm::max(a, glm::max(b, c));
        int const minX = std::max(0, static_cast<int>(std::floor(minPoint.x)));
        int const minY = std::max(0, static_cast<int>(std::floor(minPoint.y)));
        int const maxX = std::min(static_cast<int>(extent.x) - 1, static_cast<int>(std::ceil(maxPoint.x)));
        int const maxY = std::min(static_cast<int>(extent.y) - 1, static_cast<int>(std::ceil(maxPoint.y)));
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                size_t const pixelIndex = static_cast<size_t>(y) * extent.x + x;

                glm::vec2 const p(float(x) + 0.5f, float(y) + 0.5f);

                /************************************ Todo Begin *************************************/
                // TODO: Calculate barycentric coordinates of the pixel
                float w0 = glm::cross(b - a, p - a);
                float w1 = glm::cross(c - b, p - b);
                float w2 = glm::cross(a - c, p - c);

                // Discard if outside triangle
                if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f) {
                    continue;
                }

                // Normalize barycentric coordinates
                float invArea = 1.0f / area;
                float b0 = w0 * invArea;
                float b1 = w1 * invArea;
                float b2 = w2 * invArea;
                /************************************* Todo End **************************************/

                /************************************ Todo Begin *************************************/
                // TODO: Perform depth test
                float depth = b0 * triangle.vertices[0].screenAndInvW.z
                            + b1 * triangle.vertices[1].screenAndInvW.z
                            + b2 * triangle.vertices[2].screenAndInvW.z;

                if (depth >= depthBuffer[pixelIndex]) {
                    continue;
                }
                depthBuffer[pixelIndex] = depth;
                /************************************* Todo End **************************************/

                /************************************ Todo Begin *************************************/
                // TODO: Shade the pixel with pbrShade (perspective-correct interpolation)
                float invW0 = triangle.vertices[0].screenAndInvW.w;
                float invW1 = triangle.vertices[1].screenAndInvW.w;
                float invW2 = triangle.vertices[2].screenAndInvW.w;

                float pw0 = b0 * invW0;
                float pw1 = b1 * invW1;
                float pw2 = b2 * invW2;
                float pwSum = pw0 + pw1 + pw2;
                float invPwSum = 1.0f / pwSum;

                float c0 = pw0 * invPwSum;
                float c1 = pw1 * invPwSum;
                float c2 = pw2 * invPwSum;

                glm::vec3 worldPosition =
                    c0 * triangle.vertices[0].worldPosition +
                    c1 * triangle.vertices[1].worldPosition +
                    c2 * triangle.vertices[2].worldPosition;

                glm::vec3 normal = glm::normalize(
                    c0 * triangle.vertices[0].normal +
                    c1 * triangle.vertices[1].normal +
                    c2 * triangle.vertices[2].normal
                );

                glm::vec2 uv =
                    c0 * triangle.vertices[0].uv +
                    c1 * triangle.vertices[1].uv +
                    c2 * triangle.vertices[2].uv;

                                colorBuffer[pixelIndex] = glm::vec4(pbrShade(
                    material, frame, worldPosition, normal, triangle.tangent, triangle.bitangent, uv
                ), 1.0f);
                /************************************* Todo End **************************************/
            }
        }
    }
}

CpuRenderer::CpuRenderer(uint32_t width, uint32_t height)
    : Renderer(width, height) {
    resizeBackend();
}

void CpuRenderer::resizeBackend() {
    // Keep the color and depth buffers across all draw items in a frame.
    size_t const pixels = static_cast<size_t>(m_extent.x) * m_extent.y;

    m_color.resize(pixels);
    m_depthBuffer.resize(pixels);
}

void CpuRenderer::clear() {
    std::fill(m_color.begin(), m_color.end(), m_backgroundColor);
    std::fill(m_depthBuffer.begin(), m_depthBuffer.end(), 1.0f);
}

void CpuRenderer::render(DrawItem const& drawItem, FrameState const& frame) {
    // Transform and assemble this draw item's indexed triangles.
    auto const vertices = transformVertices(*drawItem.mesh, frame);
    auto const triangles = buildScreenTriangles(vertices, drawItem.mesh->indices(), frame.extent);

    // Rasterize and shade while preserving depth and color from previous draw items.
    rasterizeTriangles(triangles, *drawItem.material, frame, m_extent, m_color, m_depthBuffer);
}

gfx::Texture2D const& CpuRenderer::outputTexture() const {
    // The app calls this once after all draw items, including empty scenes.
    m_outputTexture->update(m_color.data(), gfx::Texture::ExternalFormat::RGBA, gfx::Texture::PixelType::Float);
    return Renderer::outputTexture();
}