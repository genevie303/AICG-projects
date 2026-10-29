#version 410 core

out vec4 FragColor;

in vec2 screenTexCoord;

uniform sampler2D frame;

void main() {
    vec3 color = texture(frame, screenTexCoord).rgb;
    FragColor = vec4(pow(max(color, vec3(0.0)), vec3(1.0 / 2.2)), 1.0);
}
