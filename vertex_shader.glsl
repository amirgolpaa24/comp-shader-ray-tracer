#version 450
layout (location = 0) in vec2 aPos;  // Position attribute
layout (location = 1) in vec2 aTexCoords;  // Texture coordinate attribute

out vec2 TexCoords;  // Pass texture coordinates to the fragment shader

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 0.0, 1.0);  // Convert position to vec4 and output
}
