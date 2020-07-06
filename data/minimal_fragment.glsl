#version 330

out vec4 outColour;

uniform vec3 lightPosition;

in vec3 passFragPosition;
in vec3 passNormal;

void main() {
    vec3 colour = vec3(1.0, 0.0, 1.0);

    vec3 normal = normalize(passNormal);
    vec3 lightDirection = normalize(lightPosition - passFragPosition);

    float diff = max(dot(normal, lightDirection), 0.2);
    vec3 diffuse = colour * diff;
    
    outColour = vec4(diffuse, 1.0f);
}