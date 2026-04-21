#version 330 core

out vec4 FragColor;

// Input from the Vertex Shader
in vec2 TexCoord;

// The texture we bound in main.c (blockTexture)
uniform sampler2D texture1;

void main() {
    // Read the color from the texture at the given UV coordinate
    vec4 texColor = texture(texture1, TexCoord);
    
    // If the pixel is completely transparent, we can discard it (useful for leaves later)
    if(texColor.a < 0.1)
        discard;
        
    FragColor = texColor;
}