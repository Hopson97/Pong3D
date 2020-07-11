#version 330

out vec4 outColour;

uniform vec3 lightPosition;
uniform vec3 colour;

in vec3 passFragPosition;
in vec3 passNormal;

void main() {
    vec3 normal = normalize(passNormal);
    vec3 lightDirection = normalize(lightPosition - passFragPosition);

    float diff = max(dot(normal, lightDirection), 0.2);
    vec3 diffuse = colour * diff;
    
    outColour = vec4(diffuse, 1.0f);
}