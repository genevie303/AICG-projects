#version 410 core
layout (location = 0) out vec4 fragColor;
layout (location = 1) out uint fragRngState;

in vec2 screenTexCoord;

const int LAMBERTIAN_MATERIAL = 0;
const int METAL_MATERIAL = 1;
const int DIELECTRIC_MATERIAL = 2;

const int SPHERE_SHAPE = 0;
const int TRIANGLE_SHAPE = 1;

const int DATA_BUFFER_WIDTH = 2048;

const float INFINITY = 3.402823466e+38f;
const float FloatOneMinusEpsilon = 0.99999994f;
const float Pi = 3.14159265358979323846f;
const float Epsilon = 1e-3f;

const int windowWidth = 1200;
const int maxTraceDepth = 16;

struct Camera {
    mat4 cameraToWorld; //  the inverse matrix of view matrix
    mat4 rasterToCamera;
};

struct Ray {
    vec3 origin;
    vec3 direction;
    float tMax;
};

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texCoord;
};

struct LocalCoord {
    vec3 s;
    vec3 t;
    vec3 n;
};

struct AABB {
    vec3 min;
    vec3 max;
};

struct Sphere {
    vec3 position;
    float radius;
};

struct Triangle {
    int v[3];
};

struct TriangleData {
    Vertex vertices[3];
};

struct Material {
    int type;
    float ior;
    float fuzz;
    vec3 albedo;
};

struct Primitive {
    int type; // 0: sphere 1: triangle
    int shapeIndex;
    int materialIndex;
};

struct Hit {
    Primitive primitive;
    vec3 position;
    vec3 normal;
    int materialIndex;
};

struct BVHNode {
    AABB bounds;
    // Interior: first/second are left/right child node indices (second >= 0).
    // Leaf: first is the first primitive index; second is the negative primitive count.
    int first;
    int second;
};

uniform sampler2D RTResult;
uniform highp usampler2D oldRngState;

uniform highp uint totalSamples;
uniform highp int nPrimitives;
uniform highp int nBVHNodes;

// Scene Config
uniform samplerCube sky;
uniform sampler2D sphereBuffer;
uniform sampler2D vertexBuffer;
uniform highp isampler2D triangleIndexBuffer;
uniform sampler2D materialBuffer;
uniform highp isampler2D primitiveBuffer;
uniform highp sampler2D bvh;

uniform Camera camera;

// tracer

/**
 * Summary: initialize the origin and direction of the ray of current pixel
 * Parameters:
 *     u: random offset of the origin
 * Return: the ray of current pixel
 */
Ray generateRay(vec2 u);

/**
 * Summary: get the hit point
 * Parameters:
 *     ray: the ray
 *     t: distance between the ray origin and the hit point
 * Return: the hit point
 */
vec3 getHitPoint(in Ray ray, float t);

/**
 * Summary: trace the ray in the scene and calculate the color of pixel
 * Parameters:
 *     ray: the ray
 *     rngState: random number generator state
 * Return: the color of pixel
 */
vec3 trace(Ray ray, inout uint rngState);

/**
 * Summary: check whether the ray hit something
 * Parameters:
 *     ray: the ray
 *     hit: record the primitive, hit position, normal, and material index
 * Return: true if the ray hit something
 */
bool intersect(inout Ray ray, inout Hit hit);

/**
 * Summary: check whether the ray hit AABB
 * Parameters:
 *     ray: the ray
 *     box: the AABB that will be checked
 * Return: true if the ray hit AABB
 */
bool intersectAABB(in Ray ray, in AABB box);

/**
 * Summary: check whether the ray hit primitive
 * Parameters:
 *     ray: the ray
 *     primitive: the primitive that will be checked
 *     hit: record the primitive, hit position, normal, and material index
 * Return: true if the ray hit primitive
 */
bool intersectPrimitive(inout Ray ray, in Primitive primitive, inout Hit hit);

/**
 * Summary: check whether the ray hit sphere
 * Parameters:
 *     ray: the ray
 *     sphere: the sphere that will be checked
 *     hitPosition: the intersection position
 *     hitNormal: the normal at the intersection
 * Return: true if the ray hit sphere
 */

bool intersectSphere(inout Ray ray, Sphere sphere, out vec3 hitPosition, out vec3 hitNormal);

/**
 * Summary: check whether the ray hit triangle
 * Parameters:
 *     ray: the ray
 *     a, b, c: the triangle vertices
 *     hitPosition: the intersection position
 *     hitNormal: the interpolated normal at the intersection
 * Return: true if the ray hit triangle
 */
bool intersectTriangle(inout Ray ray,
                       Vertex a,
                       Vertex b,
                       Vertex c,
                       out vec3 hitPosition,
                       out vec3 hitNormal);

// sample
/**
*Summary: uniform sample in the hemisphere
*Parameters:
*    u: random number vector
*Return: the sample vector
*/
vec3 uniformSampleHemiSphere(vec2 u);

/**
 * Summary: cosine-weighted sample in the hemisphere
 * Parameters:
 *     u: random number vector
 * Return: the sample vector
 */
vec3 cosineWeightedSampleHemisphere(vec2 u);

/**
 * Summary: uniform sample in the sphere
 * Parameters:
 *     u: random number vector
 * Return: the sample vector
 */
vec3 uniformSampleSphere(vec2 u);

// material
/**
 * Summary: approximate the reflectance of dielectric material
 * Parameters:
 *     cosTheta: cosine value between in vector and surface normal
 *     ior : index of refraction
 * Return: the reflectance of dielectric material
 */
float fresnelSchlick(float cosTheta, float ior);

/**
 * Summary: update the ray when hit lambertian material
 * Parameters:
 *     ray: the ray
 *     hit : record the shape and material of the primitive
 * Return: true if the ray continue bounce
 */
bool lambertianScatterFunction(inout Ray ray, in Hit hit, in Material material, inout uint rngState);

/**
 * Summary: update the ray when hit metal material
 * Parameters:
 *     ray: the ray
 *     hit : record the shape and material of the primitive
 * Return: true if the ray continue bounce
 */
bool metalScatterFunction(inout Ray ray, in Hit hit, in Material material, inout uint rngState);

/**
 * Summary: update the ray when hit dielectric material
 * Parameters:
 *     ray: the ray
 *     hit : record the shape and material of the primitive
 */
void dielectricScatterFunction(inout Ray ray, in Hit hit, in Material material, inout uint rngState);

// random number generator
/**
 * Summary: advance the RNG state and get a random value in [0, 1)
 * Parameters:
 *     rngState: the state to update
 * Return: the random value
 */
float randomFloat(inout uint rngState);

/**
 * Summary: get two successive random values in [0, 1), advancing the RNG state twice
 * Parameters:
 *     rngState: the state to update
 * Return: the random vector
 */
vec2 randomVec2(inout uint rngState);

LocalCoord createLocalCoord(in vec3 normal);
vec3 toLocal(LocalCoord coord, vec3 v);
vec3 toWorld(LocalCoord coord, vec3 v);

void swap(inout float a, inout float b);
vec2 getSampleIdx(sampler2D data, int idx);
void getVec3FromTexture(sampler2D data, vec2 texCoord, out vec3 v);
void getVec4FromTexture(sampler2D data, vec2 texCoord, out vec4 v);

/**
 * Summary: get material data
 * Parameters:
 *     data: buffer of data
 *     idx : index of data
 *     material: store the data
 * Return: the material data
 * Usage: getMaterialData(materialBuffer, hit.materialIndex, material)
 */
void getMaterialData(sampler2D data, int idx, out Material material);

/**
 * Summary: get sphere data
 * Parameters:
 *     data: buffer of data
 *     idx : index of data
 *     sphere: store the data
 * Return: the sphere data
 * Usage: getSphereData(sphereBuffer, hit.primitive.shapeIndex, sphere)
 */
void getSphereData(sampler2D data, int idx, out Sphere sphere);

/**
 * Summary: get the vertex index data of triangle
 * Parameters:
 *     data: buffer of data
 *     idx : index of data
 *     triangle: store the triangle vertex indices
 * Return: the vertex index data of triangle
 * Usage: getTriangleIndexData(triangleIndexBuffer, primitive.shapeIndex, triangle)
 */
void getTriangleIndexData(highp isampler2D data, int idx, out Triangle triangle);

/**
 * Summary: get the vertex data of triangle
 * Parameters:
 *     data: buffer of data
 *     idx : index of data
 *     triangle: store the expanded triangle vertices
 * Return: the vertex data of triangle
 * Usage: getTriangleData(vertexBuffer, triangle, triangleData)
 */
void getTriangleData(sampler2D data, Triangle triangle, out TriangleData triangleData);

/**
 * Summary: get primitive data
 * Parameters:
 *     data: buffer of data
 *     idx: index of the primitive
 *     primitive: store the primitive data
 * Return: the get primitive data
 * Usage: getPrimitiveData(primitiveBuffer, BVHNode.first, primitive);
 */
void getPrimitiveData(highp isampler2D data, int idx, out Primitive primitive);

/**
 * Summary: get BVHNode data
 * Parameters:
 *     data: buffer of data
 *     idx : index of data
 *     node: store the BVH node data
 * Return: the get BVHNode data
 * Usage: getBVHNodeData(bvh, BVHNode.first or BVHNode.second, node)
 */
void getBVHNodeData(highp sampler2D data, int idx, out BVHNode node);

void main() {
    uint rngState = texture(oldRngState, screenTexCoord).r;

    Ray ray = generateRay(randomVec2(rngState));
    vec3 color = trace(ray, rngState);

    if (totalSamples == 0u) {
        fragColor = vec4(color, 1.0);
    } else {
        vec3 average = texture(RTResult, screenTexCoord).rgb;
        fragColor = vec4(average + (color - average) / float(totalSamples + 1u), 1.0);
    }

    fragRngState = rngState;
}

Ray generateRay(vec2 u) {
    vec4 pixelPos = vec4(gl_FragCoord.x - 0.5 + u.x, gl_FragCoord.y - 0.5 + u.y, 0.0, 1.0);
    Ray ray;
    ray.origin = vec3(camera.cameraToWorld * vec4(0.0, 0.0, 0.0, 1.0));
    vec3 localRayDir = (camera.rasterToCamera * pixelPos).xyz - vec3(0.0, 0.0, 0.0);
    ray.direction = normalize(vec3(camera.cameraToWorld * vec4(localRayDir.xyz, 0.0)));
    ray.tMax = INFINITY;
    return ray;
}

vec3 getHitPoint(in Ray ray, float t) {
    return ray.origin + t * ray.direction;
}

vec3 trace(Ray ray, inout uint rngState) {
    vec3 color = vec3(1.0);
    for (int step = 0; step < maxTraceDepth; ++step) {
        Hit hit = Hit(Primitive(0, 0, 0), vec3(0.0), vec3(0.0), 0);
        if (!intersect(ray, hit)) {
            // TODO: Sample the skybox when the ray escapes the scene.
            // Hint: Sample the skybox in the ray direction.
            /*************************************** TODO BEGIN ******************************************/
            return color * texture(sky, ray.direction).rgb;
            /**************************************** TODO END *******************************************/
        }
        Material material;
        getMaterialData(materialBuffer, hit.materialIndex, material);
        color = color * material.albedo;
        if (material.type == LAMBERTIAN_MATERIAL) {
            if (!lambertianScatterFunction(ray, hit, material, rngState)) {
                break;
            }
        } else if (material.type == METAL_MATERIAL) {
            if (!metalScatterFunction(ray, hit, material, rngState)) {
                break;
            }
        } else {
            dielectricScatterFunction(ray, hit, material, rngState);
        }

    }

    return color;
}

bool intersect(inout Ray ray, inout Hit hit) {
    // TODO: Traverse the BVH to find the closest primitive hit.
    // Hint: Test node bounds with `intersectAABB` and leaf primitives with `intersectPrimitive`.
    /*************************************** TODO BEGIN ******************************************/
    if (nBVHNodes <= 0) {
        return false;
    }

    int stack[64];
    int stackSize = 0;
    int nodeIndex = 0;
    bool found = false;

    // A valid tree visits each node at most once. Bound traversal so invalid
    // resource state cannot turn into an infinite GPU loop or driver timeout.
    for (int visited = 0; visited < nBVHNodes; ++visited) {
        if (nodeIndex < 0 || nodeIndex >= nBVHNodes) {
            break;
        }

        BVHNode node;
        getBVHNodeData(bvh, nodeIndex, node);

        if (intersectAABB(ray, node.bounds)) {
            if (node.second < 0) {
                int count = -node.second;
                for (int i = 0; i < count; ++i) {
                    int primitiveIndex = node.first + i;
                    if (primitiveIndex < 0 || primitiveIndex >= nPrimitives) {
                        break;
                    }
                    Primitive primitive;
                    getPrimitiveData(primitiveBuffer, primitiveIndex, primitive);
                    if (intersectPrimitive(ray, primitive, hit)) {
                        found = true;
                    }
                }
            } else if (stackSize < 64) {
                stack[stackSize] = node.second;
                stackSize = stackSize + 1;
                nodeIndex = node.first;
                continue;
            }
        }

        if (stackSize == 0) {
            break;
        }
        stackSize = stackSize - 1;
        nodeIndex = stack[stackSize];
    }

    return found;
    /**************************************** TODO END *******************************************/
}

bool intersectAABB(in Ray ray, in AABB box) {
    // TODO: Test whether the ray intersects the axis-aligned bounding box.
    // Hint: Intersect the ray interval with the box slabs along all three axes.
    /*************************************** TODO BEGIN ******************************************/
    float tNear = 0.0;
    float tFar = ray.tMax;

    for (int axis = 0; axis < 3; ++axis) {
        // A direction component of zero means the ray is parallel to this slab,
        // so test containment instead of dividing.
        if (abs(ray.direction[axis]) < 1e-8) {
            if (ray.origin[axis] < box.min[axis] || ray.origin[axis] > box.max[axis]) {
                return false;
            }
            continue;
        }

        float invDirection = 1.0 / ray.direction[axis];
        float tEnter = (box.min[axis] - ray.origin[axis]) * invDirection;
        float tExit = (box.max[axis] - ray.origin[axis]) * invDirection;
        if (tEnter > tExit) {
            swap(tEnter, tExit);
        }

        tNear = max(tNear, tEnter);
        tFar = min(tFar, tExit);
        if (tNear > tFar) {
            return false;
        }
    }

    return true;
    /**************************************** TODO END *******************************************/
}

bool intersectPrimitive(inout Ray ray, in Primitive primitive, inout Hit hit) {
    // TODO: Intersect the ray with the selected primitive.
    // Hint: Call `intersectSphere` for spheres and `intersectTriangle` for triangles.
    /*************************************** TODO BEGIN ******************************************/
    vec3 hitPosition = vec3(0.0);
    vec3 hitNormal = vec3(0.0);
    bool foundHit = false;

    if (primitive.type == SPHERE_SHAPE) {
        Sphere sphere;
        getSphereData(sphereBuffer, primitive.shapeIndex, sphere);
        foundHit = intersectSphere(ray, sphere, hitPosition, hitNormal);
    } else if (primitive.type == TRIANGLE_SHAPE) {
        Triangle triangle;
        getTriangleIndexData(triangleIndexBuffer, primitive.shapeIndex, triangle);
        TriangleData triangleData;
        getTriangleData(vertexBuffer, triangle, triangleData);
        foundHit = intersectTriangle(ray,
                                     triangleData.vertices[0],
                                     triangleData.vertices[1],
                                     triangleData.vertices[2],
                                     hitPosition,
                                     hitNormal);
    }

    if (foundHit) {
        hit.primitive = primitive;
        hit.position = hitPosition;
        hit.normal = hitNormal;
        hit.materialIndex = primitive.materialIndex;
        return true;
    }

    return false;
    /**************************************** TODO END *******************************************/
}

bool intersectSphere(inout Ray ray, in Sphere sphere, out vec3 hitPosition, out vec3 hitNormal) {
    // TODO: Find the nearest valid intersection between the ray and the sphere.
    // Hint: Update `ray.tMax` and the hit point only when a closer hit is found.
    /*************************************** TODO BEGIN ******************************************/
    hitPosition = vec3(0.0);
    hitNormal = vec3(0.0);

    vec3 offset = ray.origin - sphere.position;
    float halfB = dot(offset, ray.direction);
    float c = dot(offset, offset) - sphere.radius * sphere.radius;
    float discriminant = halfB * halfB - c;

    if (discriminant < 0.0) {
        return false;
    }

    float sqrtDiscriminant = sqrt(discriminant);

    // Near root first; fall back to the far root for rays starting inside the sphere.
    float t = -halfB - sqrtDiscriminant;
    if (t <= Epsilon || t >= ray.tMax) {
        t = -halfB + sqrtDiscriminant;
        if (t <= Epsilon || t >= ray.tMax) {
            return false;
        }
    }

    ray.tMax = t;
    hitPosition = getHitPoint(ray, t);
    hitNormal = normalize(hitPosition - sphere.position);
    return true;
    /**************************************** TODO END *******************************************/
}

bool intersectTriangle(inout Ray ray,
                       in Vertex a,
                       in Vertex b,
                       in Vertex c,
                       out vec3 hitPosition,
                       out vec3 hitNormal) {
    // TODO: Find the nearest valid intersection between the ray and the triangle.
    // Hint: Use barycentric coordinates to reject hits outside the triangle.
    /*************************************** TODO BEGIN ******************************************/
    hitPosition = vec3(0.0);
    hitNormal = vec3(0.0);

    vec3 edge1 = b.position - a.position;
    vec3 edge2 = c.position - a.position;

    vec3 pVector = cross(ray.direction, edge2);
    float determinant = dot(edge1, pVector);

    // Both faces are tested, so only a near-zero determinant rejects the triangle.
    if (abs(determinant) < 1e-8) {
        return false;
    }

    float invDeterminant = 1.0 / determinant;
    vec3 tVector = ray.origin - a.position;

    float beta = dot(tVector, pVector) * invDeterminant;
    if (beta < 0.0 || beta > 1.0) {
        return false;
    }

    vec3 qVector = cross(tVector, edge1);
    float gamma = dot(ray.direction, qVector) * invDeterminant;
    if (gamma < 0.0 || beta + gamma > 1.0) {
        return false;
    }

    float t = dot(edge2, qVector) * invDeterminant;
    if (t <= Epsilon || t >= ray.tMax) {
        return false;
    }

    float alpha = 1.0 - beta - gamma;
    vec3 interpolatedNormal = alpha * a.normal + beta * b.normal + gamma * c.normal;

    // Fall back to the geometric normal when the interpolated vector degenerates.
    if (dot(interpolatedNormal, interpolatedNormal) < 1e-12) {
        interpolatedNormal = cross(edge1, edge2);
        if (dot(interpolatedNormal, interpolatedNormal) < 1e-12) {
            return false;
        }
    }

    ray.tMax = t;
    hitPosition = getHitPoint(ray, t);
    hitNormal = normalize(interpolatedNormal);
    return true;
    /**************************************** TODO END *******************************************/
}

// sample
vec3 uniformSampleHemiSphere(vec2 u) {
    float z = 1.0 - u.x;
    float sinTheta = sqrt(max(0.0, 1.0 - z * z));
    float phi = 2.0 * Pi * u.y;

    return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, z);
}

vec3 cosineWeightedSampleHemisphere(vec2 u) {
    float sinTheta = sqrt(u.x);
    float cosTheta = sqrt(1.0 - sinTheta * sinTheta);
    float phi = 2.0 * Pi * u.y;

    return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

vec3 uniformSampleSphere(vec2 u) {
    float cosTheta = 1.0 - 2.0 * u.x;
    float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));
    float phi = 2.0 * Pi * u.y;

    return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

// material
float fresnelSchlick(float cosTheta, float ior) {
    cosTheta = clamp(cosTheta, 0.0, 1.0);
    float r0 = (1.0 - ior) / (1.0 + ior);
    r0 = r0 * r0;

    return r0 + (1.0 - r0) * pow(1.0 - cosTheta, 5.0);
}

bool lambertianScatterFunction(inout Ray ray, in Hit hit, in Material material, inout uint rngState) {
    // TODO: Generate the next ray for a Lambertian material.
    // Hint: Update `ray.origin` and `ray.direction` using a cosine-weighted sample.
    /*************************************** TODO BEGIN ******************************************/
    LocalCoord coord = createLocalCoord(hit.normal);
    vec3 localDirection = cosineWeightedSampleHemisphere(randomVec2(rngState));
    vec3 worldDirection = toWorld(coord, localDirection);

    if (dot(worldDirection, worldDirection) < 1e-12) {
        return false;
    }

    // The albedo weight is applied by the tracing loop, so it is not applied here.
    ray.origin = hit.position + hit.normal * Epsilon;
    ray.direction = normalize(worldDirection);
    ray.tMax = INFINITY;
    return true;
    /**************************************** TODO END *******************************************/
}

bool metalScatterFunction(inout Ray ray, in Hit hit, in Material material, inout uint rngState) {
    // TODO: Generate the next ray for a metal material.
    // Hint: Use `material.fuzz` to perturb the reflected direction and update the ray.
    /*************************************** TODO BEGIN ******************************************/
    vec3 reflected = normalize(reflect(ray.direction, hit.normal));

    // A cosine-weighted sample around the mirror direction widens the lobe as fuzz grows.
    LocalCoord coord = createLocalCoord(reflected);
    vec3 perturbation = toWorld(coord, cosineWeightedSampleHemisphere(randomVec2(rngState)));
    vec3 scattered = reflected + material.fuzz * perturbation;

    if (dot(scattered, scattered) < 1e-12) {
        return false;
    }

    vec3 direction = normalize(scattered);
    if (dot(direction, hit.normal) <= 0.0) {
        return false;
    }

    ray.origin = hit.position + hit.normal * Epsilon;
    ray.direction = direction;
    ray.tMax = INFINITY;
    return true;
    /**************************************** TODO END *******************************************/
}

void dielectricScatterFunction(inout Ray ray, in Hit hit, in Material material, inout uint rngState) {
    // TODO: Generate the next ray for a dielectric material.
    // Hint: Choose reflection or refraction using Fresnel reflectance and total internal reflection.
    /*************************************** TODO BEGIN ******************************************/
    float cosIncident = dot(ray.direction, hit.normal);
    bool entering = cosIncident < 0.0;

    // Face the normal against the incident direction and pick the matching index ratio.
    vec3 facingNormal = entering ? hit.normal : -hit.normal;
    float refractionRatio = entering ? 1.0 / material.ior : material.ior;
    float fresnelCosine = entering ? -cosIncident : material.ior * cosIncident;

    vec3 refracted = refract(ray.direction, facingNormal, refractionRatio);
    bool totalInternalReflection = dot(refracted, refracted) < 1e-12;

    vec3 direction;
    vec3 origin;
    if (totalInternalReflection || randomFloat(rngState) < fresnelSchlick(fresnelCosine, material.ior)) {
        // Reflection stays on the incident side of the surface.
        direction = reflect(ray.direction, facingNormal);
        origin = hit.position + facingNormal * Epsilon;
    } else {
        // Refraction crosses the surface, so the offset points the other way.
        direction = refracted;
        origin = hit.position - facingNormal * Epsilon;
    }

    ray.origin = origin;
    ray.direction = normalize(direction);
    ray.tMax = INFINITY;
    /**************************************** TODO END *******************************************/
}

float randomFloat(inout uint rngState) {
    rngState ^= (rngState << 13);
    rngState ^= (rngState >> 17);
    rngState ^= (rngState << 5);
    return min(FloatOneMinusEpsilon, float(rngState) * (1.0 / 4294967296.0));
}

vec2 randomVec2(inout uint rngState) {
    float x = randomFloat(rngState);
    float y = randomFloat(rngState);
    return vec2(x, y);
}

LocalCoord createLocalCoord(in vec3 normal) {
    LocalCoord res;
    res.n = normalize(normal);
    if (abs(res.n.x) > abs(res.n.y)) {
        res.s = normalize(vec3(-res.n.z, 0.0, res.n.x));
    }
    else {
        res.s = normalize(vec3(0.0, -res.n.z, res.n.y));
    }
    res.t = normalize(cross(res.n, res.s));
    return res;
}

vec3 toLocal(LocalCoord coord, vec3 v) {
    return vec3(v.x * coord.s.x + v.y * coord.s.y + v.z * coord.s.z,
                v.x * coord.t.x + v.y * coord.t.y + v.z * coord.t.z,
                v.x * coord.n.x + v.y * coord.n.y + v.z * coord.n.z);
}
vec3 toWorld(LocalCoord coord, vec3 v) {
    return v.x * coord.s + v.y * coord.t + v.z * coord.n;
}

void swap(inout float a, inout float b) {
    float tmp = a;
    a = b;
    b = tmp;
}

vec2 getISampleIdx(highp isampler2D data, int idx) {
    ivec2 texSize = textureSize(data, 0);
    int x = idx % texSize.x;
    int y = idx / texSize.x;
    return vec2((float(x) + 0.5) / float(texSize.x), (float(y) + 0.5) / float(texSize.y));
}

vec2 getSampleIdx(sampler2D data, int idx) {
    ivec2 texSize = textureSize(data, 0);
    int x = idx % texSize.x;
    int y = idx / texSize.x;
    return vec2((float(x) + 0.5) / float(texSize.x), (float(y) + 0.5) / float(texSize.y));
}

void getVec3FromTexture(sampler2D data, vec2 texCoord, out vec3 v) {
    v = texture(data, texCoord).rgb;
}

void getVec4FromTexture(sampler2D data, vec2 texCoord, out vec4 v) {
    v = texture(data, texCoord);
}

void getMaterialData(sampler2D data, int idx, out Material material) {
    idx *= 2;
    vec3 v;
    getVec3FromTexture(data, getSampleIdx(data, idx), v);
    material.type = int(v.x);
    material.ior = v.y;
    material.fuzz = v.z;
    getVec3FromTexture(data, getSampleIdx(data, idx + 1), material.albedo);
}

void getSphereData(sampler2D data, int idx, out Sphere sphere) {
    vec4 v;
    getVec4FromTexture(data, getSampleIdx(data, idx), v);
    sphere.position = v.xyz;
    sphere.radius = v.w;
}

void getTriangleIndexData(highp isampler2D data, int idx, out Triangle triangle) {
    ivec3 v = texture(data, getISampleIdx(data, idx)).xyz;
    triangle.v[0] = v.x;
    triangle.v[1] = v.y;
    triangle.v[2] = v.z;
}

void getTriangleData(sampler2D data, Triangle triangle, out TriangleData triangleData) {
    vec4 v[2];
    for (int i = 0; i < 3; ++i) {
        int vid = triangle.v[i] * 2;
        getVec4FromTexture(data, getSampleIdx(data, vid), v[0]);
        getVec4FromTexture(data, getSampleIdx(data, vid + 1), v[1]);
        triangleData.vertices[i].position = v[0].xyz;
        triangleData.vertices[i].normal = vec3(v[0].w, v[1].x, v[1].y);
        triangleData.vertices[i].texCoord = v[1].zw;
    }
}

void getPrimitiveData(highp isampler2D data, int idx, out Primitive primitive) {
    ivec3 v = texture(data, getISampleIdx(data, idx)).xyz;
    primitive.type = v.x;
    primitive.shapeIndex = v.y;
    primitive.materialIndex = v.z;
}

void getBVHNodeData(highp sampler2D data, int idx, out BVHNode node) {
    vec4 firstTexel;
    vec4 secondTexel;
    getVec4FromTexture(data, getSampleIdx(data, idx * 2), firstTexel);
    getVec4FromTexture(data, getSampleIdx(data, idx * 2 + 1), secondTexel);
    node.bounds.min = firstTexel.xyz;
    node.bounds.max = vec3(firstTexel.w, secondTexel.xy);
    node.first = int(secondTexel.z);
    node.second = int(secondTexel.w);
}