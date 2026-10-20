#version 410 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUv;

uniform mat4 uModel;
uniform mat4 uModelViewProjection;
uniform mat3 uNormalTransform;

// Captured before fixed-function clipping, in the ClipVertex record order.
out precise vec4 tfClip;
out precise vec3 tfWorldPosition;
out precise vec3 tfNormal;
out vec2 tfUv;

void main() {
    // TODO: apply mvp transformation for each vertices
    /*************************************** TODO BEGIN ******************************************/
    tfClip = uModelViewProjection * vec4(aPosition, 1.0);
    tfWorldPosition = vec3(uModel * vec4(aPosition, 1.0));
    tfNormal = uNormalTransform * aNormal;
    tfUv = aUv;
    gl_Position = tfClip;
    /**************************************** TODO END *******************************************/
}
