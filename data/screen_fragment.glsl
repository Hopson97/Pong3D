#version 330

in vec2 passTexCoord;
out vec4 outColour;

uniform sampler2D tex;
uniform sampler2D tex2;

void main() {
    vec3 colour1 = texture(tex, passTexCoord).rgb;
    vec3 colour2 = texture(tex2, passTexCoord).rgb;
    outColour = vec4(colour1 + colour2, 1.0);
}