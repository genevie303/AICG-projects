#include "cpu_ray_tracer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <utility>

constexpr float kEpsilon = 1e-3f;
constexpr int kMaxTraceDepth = 16;

struct LocalCoord {
    glm::vec3 s;
    glm::vec3 t;
    glm::vec3 n;
};

// Advances the RNG state and returns a random value in [0, 1).
static float randomFloat(uint32_t& rngState) {
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return std::min(0.99999994f, static_cast<float>(rngState) * (1.0f / 4294967296.0f));
}

// Returns two successive random values in [0, 1), advancing the RNG state twice.
static glm::vec2 randomVec2(uint32_t& rngState) {
    return {randomFloat(rngState), randomFloat(rngState)};
}

static Ray generateRay(glm::vec2 pixelPosition, glm::mat4 const& cameraToWorld, glm::mat4 const& rasterToCamera) {
    auto const cameraOrigin = glm::vec3(cameraToWorld * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});
    auto const cameraDirection = glm::vec3(rasterToCamera * glm::vec4{pixelPosition, 0.0f, 1.0f});
    return {cameraOrigin, glm::normalize(glm::vec3(cameraToWorld * glm::vec4{cameraDirection, 0.0f}))};
}

static glm::vec3 cosineWeightedSampleHemisphere(glm::vec2 u) {
    auto const sinTheta = std::sqrt(u.x);
    auto const cosTheta = std::sqrt(1.0f - sinTheta * sinTheta);
    auto const phi = 2.0f * std::numbers::pi_v<float> * u.y;
    return {std::cos(phi) * sinTheta, std::sin(phi) * sinTheta, cosTheta};
}

static LocalCoord createLocalCoord(glm::vec3 normal) {
    LocalCoord coord;
    coord.n = glm::normalize(normal);
    coord.s = glm::normalize(std::abs(coord.n.x) > std::abs(coord.n.y) ? glm::vec3{-coord.n.z, 0.0f, coord.n.x}
                                                                       : glm::vec3{0.0f, -coord.n.z, coord.n.y});
    coord.t = glm::normalize(glm::cross(coord.n, coord.s));
    return coord;
}

static glm::vec3 toWorld(LocalCoord const& coord, glm::vec3 const& v) {
    return v.x * coord.s + v.y * coord.t + v.z * coord.n;
}

static float fresnelSchlick(float cosTheta, float ior) {
    cosTheta = std::clamp(cosTheta, 0.0f, 1.0f);
    auto r0 = (1.0f - ior) / (1.0f + ior);
    r0 *= r0;
    return r0 + (1.0f - r0) * std::pow(1.0f - cosTheta, 5.0f);
}

static bool intersectAABB(Ray const& ray, AABB const& box) {
    // TODO: Perform ray-AABB intersection.
    /**************************************** Todo Begin *****************************************/
    float tNear = 0.0f;
    float tFar = ray.tMax;

    for (int axis = 0; axis < 3; ++axis) {
        // A direction component of zero means the ray is parallel to this slab.
        // Dividing would produce inf or NaN, so test containment instead.
        if (std::abs(ray.direction[axis]) < 1e-8f) {
            if (ray.origin[axis] < box.min[axis] || ray.origin[axis] > box.max[axis]) {
                return false;
            }
            continue;
        }

        auto const invDirection = 1.0f / ray.direction[axis];
        auto tEnter = (box.min[axis] - ray.origin[axis]) * invDirection;
        auto tExit = (box.max[axis] - ray.origin[axis]) * invDirection;
        if (tEnter > tExit) {
            std::swap(tEnter, tExit);
        }

        tNear = std::max(tNear, tEnter);
        tFar = std::min(tFar, tExit);
        if (tNear > tFar) {
            return false;
        }
    }

    return true;
    /***************************************** Todo End ******************************************/
}

static bool intersectSphere(Ray& ray, Sphere const& sphere, glm::vec3& hitPosition, glm::vec3& hitNormal) {
    // TODO: Find the nearest valid intersection between the ray and the sphere.
    // Hint: Update `ray.tMax` when a closer hit is found.
    /**************************************** Todo Begin *****************************************/
    auto const offset = ray.origin - sphere.position;
    auto const halfB = glm::dot(offset, ray.direction);
    auto const c = glm::dot(offset, offset) - sphere.radius * sphere.radius;
    auto const discriminant = halfB * halfB - c;

    if (discriminant < 0.0f) {
        return false;
    }

    auto const sqrtDiscriminant = std::sqrt(discriminant);

    // Near root first; fall back to the far root for rays starting inside the sphere.
    auto t = -halfB - sqrtDiscriminant;
    if (t <= kEpsilon || t >= ray.tMax) {
        t = -halfB + sqrtDiscriminant;
        if (t <= kEpsilon || t >= ray.tMax) {
            return false;
        }
    }

    ray.tMax = t;
    hitPosition = ray.at(t);
    hitNormal = glm::normalize(hitPosition - sphere.position);
    return true;
    /***************************************** Todo End ******************************************/
}

static bool intersectTriangle(
    Ray& ray, Vertex const& a, Vertex const& b, Vertex const& c, glm::vec3& hitPosition, glm::vec3& hitNormal) {
    // TODO: Find the nearest valid intersection between the ray and the triangle.
    // Hint: Update `ray.tMax` when a closer hit is found.
    /**************************************** Todo Begin *****************************************/
    auto const edge1 = b.position - a.position;
    auto const edge2 = c.position - a.position;

    auto const pVector = glm::cross(ray.direction, edge2);
    auto const determinant = glm::dot(edge1, pVector);

    // Both faces are tested, so only a near-zero determinant rejects the triangle.
    if (std::abs(determinant) < 1e-8f) {
        return false;
    }

    auto const invDeterminant = 1.0f / determinant;
    auto const tVector = ray.origin - a.position;

    auto const beta = glm::dot(tVector, pVector) * invDeterminant;
    if (beta < 0.0f || beta > 1.0f) {
        return false;
    }

    auto const qVector = glm::cross(tVector, edge1);
    auto const gamma = glm::dot(ray.direction, qVector) * invDeterminant;
    if (gamma < 0.0f || beta + gamma > 1.0f) {
        return false;
    }

    auto const t = glm::dot(edge2, qVector) * invDeterminant;
    if (t <= kEpsilon || t >= ray.tMax) {
        return false;
    }

    auto const alpha = 1.0f - beta - gamma;
    auto interpolatedNormal = alpha * a.normal + beta * b.normal + gamma * c.normal;

    // Fall back to the geometric normal when the interpolated vector degenerates.
    if (glm::dot(interpolatedNormal, interpolatedNormal) < 1e-12f) {
        interpolatedNormal = glm::cross(edge1, edge2);
        if (glm::dot(interpolatedNormal, interpolatedNormal) < 1e-12f) {
            return false;
        }
    }

    ray.tMax = t;
    hitPosition = ray.at(t);
    hitNormal = glm::normalize(interpolatedNormal);
    return true;
    /***************************************** Todo End ******************************************/
}

CpuRayTracer::CpuRayTracer(uint32_t width, uint32_t height)
    : RayTracer{width, height} {
    createFrameResources();
    reset();
}

void CpuRayTracer::resize(uint32_t width, uint32_t height) {
    if (!updateExtent(width, height)) {
        return;
    }
    createFrameResources();
    reset();
}

void CpuRayTracer::createFrameResources() {
    auto const count = static_cast<size_t>(m_width) * m_height;
    m_rngStates.resize(count);
    for (size_t i = 0; i < m_rngStates.size(); ++i) {
        m_rngStates[i] = 1664525u * static_cast<uint32_t>(i) + 1013904223u;
    }

    m_average.assign(count, glm::vec3{0.0f});
    m_frame = std::make_unique<gfx::Texture2D>(
        gfx::Texture2D::Specification{
            .width = m_width,
            .height = m_height,
            .format = gfx::Texture::Format::RGB32F,
            .pixelFormat = gfx::Texture::ExternalFormat::RGB,
            .pixelType = gfx::Texture::PixelType::Float
        },
        m_average.data());
}

void CpuRayTracer::reset() {
    RayTracer::reset();
    std::fill(m_average.begin(), m_average.end(), glm::vec3{0.0f});
}

void CpuRayTracer::render(glm::mat4 const& cameraToWorld, glm::mat4 const& rasterToCamera) {
    if (m_average.empty() || !m_scene || m_scene->nodes().empty()) {
        return;
    }

    auto const sampleWeight = 1.0f / static_cast<float>(m_completedSamples + 1);
    for (size_t index = 0; index < m_average.size(); ++index) {
        auto& rngState = m_rngStates[index];

        auto const pixelPosition =
            glm::vec2{static_cast<float>(index % m_width), static_cast<float>(index / m_width)} + randomVec2(rngState);
        auto const ray = generateRay(pixelPosition, cameraToWorld, rasterToCamera);
        glm::vec3 color = trace(ray, rngState);

        auto& average = m_average[index];
        average += (color - average) * sampleWeight;
    }

    ++m_completedSamples;
    m_frame->update(m_average.data(), gfx::Texture::ExternalFormat::RGB, gfx::Texture::PixelType::Float);
}

bool CpuRayTracer::intersectPrimitive(Ray& ray, Primitive const& primitive, Hit& hit) const {
    glm::vec3 hitPosition;
    glm::vec3 hitNormal;
    bool foundHit = false;
    if (primitive.type == Primitive::Type::Sphere) {
        auto const& sphere = m_scene->spheres()[primitive.shapeIndex];
        foundHit = intersectSphere(ray, sphere, hitPosition, hitNormal);
    }
    else if (primitive.type == Primitive::Type::Triangle) {
        auto const& triangle = m_scene->triangles()[primitive.shapeIndex];
        auto const& a = m_scene->vertices()[triangle.v[0]];
        auto const& b = m_scene->vertices()[triangle.v[1]];
        auto const& c = m_scene->vertices()[triangle.v[2]];
        foundHit = intersectTriangle(ray, a, b, c, hitPosition, hitNormal);
    }

    if (foundHit) {
        hit.primitive = primitive;
        hit.position = hitPosition;
        hit.normal = hitNormal;
        hit.materialIndex = primitive.materialIndex;
        return true;
    }
    else {
        return false;
    }
}

bool CpuRayTracer::intersect(Ray& ray, Hit& hit) const {
    std::array<int, 64> stack;
    size_t stackSize = 0;
    int nodeIndex = 0;
    bool found = false;
    while (true) {
        auto const& node = m_scene->nodes()[nodeIndex];
        if (intersectAABB(ray, node.bounds)) {
            if (node.isLeaf()) {
                for (int i = 0; i < -node.second; ++i) {
                    found |= intersectPrimitive(ray, m_scene->primitives()[node.first + i], hit);
                }
            }
            else {
                stack[stackSize++] = node.second;
                nodeIndex = node.first;
                continue;
            }
        }
        if (stackSize == 0) {
            break;
        }
        nodeIndex = stack[--stackSize];
    }

    return found;
}

bool CpuRayTracer::lambertianScatterFunction(Ray& ray,
                                             Hit const& hit,
                                             Material const& material,
                                             uint32_t& rngState) const {
    // TODO: Generate the next ray for a Lambertian material.
    // Hint: Update `ray.origin` and `ray.direction` using a cosine-weighted sample.
    /**************************************** Todo Begin *****************************************/
    auto const coord = createLocalCoord(hit.normal);
    auto const localDirection = cosineWeightedSampleHemisphere(randomVec2(rngState));
    auto const worldDirection = toWorld(coord, localDirection);

    if (glm::dot(worldDirection, worldDirection) < 1e-12f) {
        return false;
    }

    // The albedo weight is applied by the tracing loop, so it is not applied here.
    ray.origin = hit.position + hit.normal * kEpsilon;
    ray.direction = glm::normalize(worldDirection);
    ray.tMax = std::numeric_limits<float>::max();
    return true;
    /***************************************** Todo End ******************************************/
}

bool CpuRayTracer::metalScatterFunction(Ray& ray, Hit const& hit, Material const& material, uint32_t& rngState) const {
    // TODO: Generate the next ray for a metal material.
    // Hint: Use `material.fuzz` to perturb the reflected direction and update the ray.
    /**************************************** Todo Begin *****************************************/
    auto const reflected = glm::normalize(glm::reflect(ray.direction, hit.normal));

    // A cosine-weighted sample around the mirror direction widens the lobe as fuzz grows.
    auto const coord = createLocalCoord(reflected);
    auto const perturbation = toWorld(coord, cosineWeightedSampleHemisphere(randomVec2(rngState)));
    auto const scattered = reflected + material.fuzz * perturbation;

    if (glm::dot(scattered, scattered) < 1e-12f) {
        return false;
    }

    auto const direction = glm::normalize(scattered);
    if (glm::dot(direction, hit.normal) <= 0.0f) {
        return false;
    }

    ray.origin = hit.position + hit.normal * kEpsilon;
    ray.direction = direction;
    ray.tMax = std::numeric_limits<float>::max();
    return true;
    /***************************************** Todo End ******************************************/
}

void CpuRayTracer::dielectricScatterFunction(Ray& ray,
                                             Hit const& hit,
                                             Material const& material,
                                             uint32_t& rngState) const {
    // TODO: Generate the next ray for a dielectric material.
    // Hint: Choose reflection or refraction using Fresnel reflectance and total internal reflection.
    /**************************************** Todo Begin *****************************************/
    auto const cosIncident = glm::dot(ray.direction, hit.normal);
    auto const entering = cosIncident < 0.0f;

    // Face the normal against the incident direction and pick the matching index ratio.
    auto const facingNormal = entering ? hit.normal : -hit.normal;
    auto const refractionRatio = entering ? 1.0f / material.ior : material.ior;
    auto const fresnelCosine = entering ? -cosIncident : material.ior * cosIncident;

    auto const refracted = glm::refract(ray.direction, facingNormal, refractionRatio);
    auto const totalInternalReflection = glm::dot(refracted, refracted) < 1e-12f;

    glm::vec3 direction;
    glm::vec3 origin;
    if (totalInternalReflection || randomFloat(rngState) < fresnelSchlick(fresnelCosine, material.ior)) {
        // Reflection stays in the incident medium.
        direction = glm::reflect(ray.direction, facingNormal);
        origin = hit.position + facingNormal * kEpsilon;
    }
    else {
        // Refraction crosses the surface, so the offset points the other way.
        direction = refracted;
        origin = hit.position - facingNormal * kEpsilon;
    }

    ray.origin = origin;
    ray.direction = glm::normalize(direction);
    ray.tMax = std::numeric_limits<float>::max();
    /***************************************** Todo End ******************************************/
}

glm::vec3 CpuRayTracer::trace(Ray ray, uint32_t& rngState) const {
    glm::vec3 color{1.0f};
    for (int step = 0; step < kMaxTraceDepth; ++step) {
        Hit hit;
        if (!intersect(ray, hit)) {
            // TODO: Sample the skybox when the ray escapes the scene.
            // Hint: Sample the skybox in the ray direction.
            /**************************************** Todo Begin *****************************************/
            return color * m_skybox->sample(ray.direction);
            /***************************************** Todo End ******************************************/
        }
        auto const& material = m_scene->materials()[hit.materialIndex];
        color *= material.albedo;

        if (material.type == Material::Type::Lambertian) {
            if (!lambertianScatterFunction(ray, hit, material, rngState)) {
                break;
            }
        }
        else if (material.type == Material::Type::Metal) {
            if (!metalScatterFunction(ray, hit, material, rngState)) {
                break;
            }
        }
        else {
            dielectricScatterFunction(ray, hit, material, rngState);
        }
    }

    return color;
}