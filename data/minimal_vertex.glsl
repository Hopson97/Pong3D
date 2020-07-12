#version 330

layout (location = 0) in vec3 inVertexCoord;
layout (location = 1) in vec3 inNormal;

uniform mat4 modelMatrix;
uniform mat4 projectionViewMatrix;

out vec3 passFragPosition;
out vec3 passNormal;

void main() {
    vec4 worldPos = modelMatrix * vec4(inVertexCoord, 1.0);
    //worldPos.y += (worldPos.x / 32) * (worldPos.x / 32);
    //worldPos.y += (worldPos.z / 32) * (worldPos.z / 32);
    gl_Position = projectionViewMatrix * worldPos;

    passFragPosition = vec3(modelMatrix * vec4(inVertexCoord, 1.0));
    passNormal = mat3(modelMatrix) * inNormal;
}