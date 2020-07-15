#version 330

in vec2 passTexCoord;
out vec4 outColour;

uniform sampler2D tex;

void main() {
    vec4 colour = texture(tex, passTexCoord);
    
    float brightness = (colour.r * 1.5) + colour.g + colour.b;
    outColour = colour;
}