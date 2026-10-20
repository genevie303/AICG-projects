#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 21) out;

in VertexData {
    vec4 clip;
    vec3 worldPosition;
    vec3 normal;
    vec2 uv;
} vertexData[];

flat out vec4 gScreenAndInvW[3];
flat out vec3 gWorld[3];
flat out vec3 gNormal[3];
flat out vec2 gUv[3];

uniform vec2 uExtent;

struct ClipVertex {
    vec4 clip;
    vec3 worldPosition;
    vec3 normal;
    vec2 uv;
};

struct ScreenVertex {
    vec4 screenAndInvW;
    vec3 worldPosition;
    vec3 normal;
    vec2 uv;
};

int clipAgainst(inout ClipVertex polygon[9], int count, vec4 plane) {
    ClipVertex result[9];
    int outCount = 0;
    for (int i = 0; i < count; ++i) {
        ClipVertex curr = polygon[i];
        ClipVertex next = polygon[(i + 1) % count];
        float dCurr = dot(plane, curr.clip);
        float dNext = dot(plane, next.clip);
        if (dCurr >= 0.0) {
            result[outCount++] = curr;
        }
        if ((dCurr >= 0.0) != (dNext >= 0.0)) {
            float t = dCurr / (dCurr - dNext);
            ClipVertex intersect;
            intersect.clip          = mix(curr.clip, next.clip, t);
            intersect.worldPosition = mix(curr.worldPosition, next.worldPosition, t);
            intersect.normal        = mix(curr.normal, next.normal, t);
            intersect.uv            = mix(curr.uv, next.uv, t);
            result[outCount++] = intersect;
        }
    }
    for (int i = 0; i < outCount; ++i) polygon[i] = result[i];
    return outCount;
}

void main() {
    ClipVertex polygon[9];
    int count = 3;
    for (int i = 0; i < 3; ++i) {
        polygon[i].clip = vertexData[i].clip;
        polygon[i].worldPosition = vertexData[i].worldPosition;
        polygon[i].normal = vertexData[i].normal;
        polygon[i].uv = vertexData[i].uv;
    }

    count = clipAgainst(polygon, count, vec4(1, 0, 0, 1));
    count = clipAgainst(polygon, count, vec4(-1, 0, 0, 1));
    count = clipAgainst(polygon, count, vec4(0, 1, 0, 1));
    count = clipAgainst(polygon, count, vec4(0, -1, 0, 1));
    count = clipAgainst(polygon, count, vec4(0, 0, 1, 1));
    count = clipAgainst(polygon, count, vec4(0, 0, -1, 1));

    for (int fan = 1; fan + 1 < count; ++fan) {
        ClipVertex triangle[3] = ClipVertex[3](polygon[0], polygon[fan], polygon[fan + 1]);
        ScreenVertex screenTriangle[3];

        for (int i = 0; i < 3; ++i) {
            float invW = 1.0 / triangle[i].clip.w;
            vec3 ndc   = triangle[i].clip.xyz * invW;
            screenTriangle[i].screenAndInvW = vec4(
                (ndc.x * 0.5 + 0.5) * uExtent.x,
                (ndc.y * 0.5 + 0.5) * uExtent.y,
                ndc.z * 0.5 + 0.5,
                invW
            );
            screenTriangle[i].worldPosition = triangle[i].worldPosition;
            screenTriangle[i].normal        = triangle[i].normal;
            screenTriangle[i].uv            = triangle[i].uv;
        }

        vec2 bmin = min(screenTriangle[0].screenAndInvW.xy,
                    min(screenTriangle[1].screenAndInvW.xy,
                        screenTriangle[2].screenAndInvW.xy));
        vec2 bmax = max(screenTriangle[0].screenAndInvW.xy,
                    max(screenTriangle[1].screenAndInvW.xy,
                        screenTriangle[2].screenAndInvW.xy));

        for (int i = 0; i < 3; ++i) {
            gScreenAndInvW[i] = screenTriangle[i].screenAndInvW;
            gWorld[i]         = screenTriangle[i].worldPosition;
            gNormal[i]        = screenTriangle[i].normal;
            gUv[i]            = screenTriangle[i].uv;
        }

        vec2 invExtent = 1.0 / uExtent;
        vec2 corners[4];
        corners[0] = vec2(bmin.x, bmin.y);
        corners[1] = vec2(bmax.x, bmin.y);
        corners[2] = vec2(bmin.x, bmax.y);
        corners[3] = vec2(bmax.x, bmax.y);

        for (int i = 0; i < 4; ++i) {
            gl_Position = vec4(corners[i] * invExtent * 2.0 - 1.0, 0.0, 1.0);
            EmitVertex();
        }
        EndPrimitive();
    }
}