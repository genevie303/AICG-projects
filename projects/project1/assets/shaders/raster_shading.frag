#version 410 core

flat in vec4 gScreenAndInvW[3];
flat in vec3 gWorld[3];
flat in vec3 gNormal[3];
flat in vec2 gUv[3];

layout(location = 0) out vec4 outColor;

uniform vec3 uCameraPosition;
uniform vec3 uLightDirection;
uniform vec3 uLightColor;
uniform float uLightIntensity;
uniform vec4 uBaseColorFactor;
uniform vec3 uEmissiveFactor;
uniform float uMetallicFactor;
uniform float uRoughnessFactor;
uniform float uOcclusionStrength;
uniform bool uHasTexture[5];
uniform sampler2D uTextures[5];

vec3 srgbToLinear(vec3 value) {
    vec3 result;
    for (int i = 0; i < 3; ++i) {
        result[i] = value[i] <= 0.04045 ? value[i] / 12.92 : pow((value[i] + 0.055) / 1.055, 2.4);
    }
    return result;
}

vec3 pbrShade(vec3 worldPosition, vec3 normal, vec3 tangent, vec3 bitangent, vec2 uv) {
    const float kPi = 3.14159265358979323846;
    vec4 baseColor = uBaseColorFactor;
    if (uHasTexture[0]) {
        vec4 sampledColor = textureLod(uTextures[0], uv, 0.0);
        baseColor *= vec4(srgbToLinear(sampledColor.rgb), sampledColor.a);
    }
    float roughness = uRoughnessFactor;
    float metallic = uMetallicFactor;
    if (uHasTexture[1]) {
        vec4 sampledColor = textureLod(uTextures[1], uv, 0.0);
        roughness *= sampledColor.g;
        metallic *= sampledColor.b;
    }
    roughness = clamp(roughness, 0.045, 1.0);
    metallic = clamp(metallic, 0.0, 1.0);

    normal = normalize(normal);
    if (uHasTexture[2]) {
        vec3 tangentNormal = textureLod(uTextures[2], uv, 0.0).xyz * 2.0 - 1.0;
        vec3 orthogonalTangent = normalize(tangent - normal * dot(normal, tangent));
        normal = normalize(mat3(orthogonalTangent, normalize(bitangent), normal) * tangentNormal);
    }

    vec3 viewDirection = normalize(uCameraPosition - worldPosition);
    vec3 lightDirection = normalize(-uLightDirection);
    vec3 halfway = normalize(viewDirection + lightDirection);
    float nDotL = max(dot(normal, lightDirection), 0.0);
    float nDotV = max(dot(normal, viewDirection), 0.0);
    float nDotH = max(dot(normal, halfway), 0.0);
    float vDotH = max(dot(viewDirection, halfway), 0.0);
    float alphaRoughness = roughness * roughness;
    float alphaSquared = alphaRoughness * alphaRoughness;
    float denominator = nDotH * nDotH * (alphaSquared - 1.0) + 1.0;
    float distribution = alphaSquared / max(kPi * denominator * denominator, 1e-5);

    float attenuationL = 2.0 * nDotL / (nDotL + sqrt(alphaSquared + (1.0 - alphaSquared) * nDotL * nDotL));
    float attenuationV = 2.0 * nDotV / (nDotV + sqrt(alphaSquared + (1.0 - alphaSquared) * nDotV * nDotV));
    float geometry = attenuationL * attenuationV;

    vec3 f0 = mix(vec3(0.04), baseColor.rgb, metallic);
    vec3 fresnel = f0 + (vec3(1.0) - f0) * pow(1.0 - vDotH, 5.0);
    vec3 specular = fresnel * distribution * geometry / max(4.0 * nDotL * nDotV, 1e-4);
    vec3 diffuse = (vec3(1.0) - fresnel) * (1.0 - metallic) * baseColor.rgb / kPi;
    vec3 color = (diffuse + specular) * uLightColor * uLightIntensity * nDotL;
    if (uHasTexture[3]) {
        float occlusion = textureLod(uTextures[3], uv, 0.0).r;
        color *= 1.0 + uOcclusionStrength * (occlusion - 1.0);
    }
    vec3 emissive = uEmissiveFactor;
    if (uHasTexture[4]) {
        emissive *= srgbToLinear(textureLod(uTextures[4], uv, 0.0).rgb);
    }
    return color + emissive;
}

void main() {
    vec2 p = gl_FragCoord.xy;
    vec2 a = gScreenAndInvW[0].xy;
    vec2 b = gScreenAndInvW[1].xy;
    vec2 c = gScreenAndInvW[2].xy;
    float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);

    /*************************************** TODO BEGIN ******************************************/
    float w0 = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
    float w1 = (c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x);
    float w2 = (a.x - c.x) * (p.y - c.y) - (a.y - c.y) * (p.x - c.x);
    if (area > 0.0) {
        if (w0 < 0.0 || w1 < 0.0 || w2 < 0.0) discard;
    } else {
        if (w0 > 0.0 || w1 > 0.0 || w2 > 0.0) discard;
    }
    /**************************************** TODO END *******************************************/

    /*************************************** TODO BEGIN ******************************************/
    float b0 = w0 / area;
    float b1 = w1 / area;
    float b2 = w2 / area;
    gl_FragDepth = b0 * gScreenAndInvW[0].z
                 + b1 * gScreenAndInvW[1].z
                 + b2 * gScreenAndInvW[2].z;
    /**************************************** TODO END *******************************************/

    /*************************************** TODO BEGIN ******************************************/
    float invW0 = gScreenAndInvW[0].w;
    float invW1 = gScreenAndInvW[1].w;
    float invW2 = gScreenAndInvW[2].w;
    float pw0 = b0 * invW0;
    float pw1 = b1 * invW1;
    float pw2 = b2 * invW2;
    float pwSum = pw0 + pw1 + pw2;
    float c0 = pw0 / pwSum;
    float c1 = pw1 / pwSum;
    float c2 = pw2 / pwSum;

    vec3 worldPosition = c0 * gWorld[0] + c1 * gWorld[1] + c2 * gWorld[2];
    vec3 normal        = c0 * gNormal[0] + c1 * gNormal[1] + c2 * gNormal[2];
    vec2 uv            = c0 * gUv[0]     + c1 * gUv[1]     + c2 * gUv[2];

    vec3 dp1  = gWorld[1] - gWorld[0];
    vec3 dp2  = gWorld[2] - gWorld[0];
    vec2 duv1 = gUv[1] - gUv[0];
    vec2 duv2 = gUv[2] - gUv[0];
    float det = duv1.x * duv2.y - duv1.y * duv2.x;
    vec3 tangent;
    vec3 bitangent;
    if (abs(det) > 1e-6) {
        float invDet = 1.0 / det;
        tangent   = normalize((dp1 * duv2.y - dp2 * duv1.y) * invDet);
        bitangent = normalize((dp2 * duv1.x - dp1 * duv2.x) * invDet);
    } else {
        vec3 up = abs(normal.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
        tangent   = normalize(cross(up, normal));
        bitangent = cross(normal, tangent);
    }
    /**************************************** TODO END *******************************************/

    outColor = vec4(pbrShade(worldPosition, normal, tangent, bitangent, uv), 1.0);
}