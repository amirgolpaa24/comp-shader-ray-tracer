#version 450
in vec2 TexCoords;  // Texture coordinates from vertex shader
out vec4 FragColor;  // Output color of the pixel

uniform sampler2D screenTexture;  // The texture to sample

void main() {
    FragColor = texture(screenTexture, TexCoords);  // Sample the texture at the coordinates
}
