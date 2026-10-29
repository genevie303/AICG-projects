#version 410 core

out vec2 screenTexCoord;

void main() {
    const vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));
    const vec2 texCoords[3] = vec2[](
        vec2(0.0, 0.0), vec2(2.0, 0.0), vec2(0.0, 2.0));

    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
    screenTexCoord = texCoords[gl_VertexID];
}
