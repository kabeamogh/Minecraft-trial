#version 330 core

// These locations must match your glVertexAttribPointer calls in chunk.c!
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

// Output to the Fragment Shader
out vec2 TexCoord;

// The matrices we send from main.c
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // We calculate the final 3D position by multiplying the matrices
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // Pass the texture coordinates down the pipeline
    TexCoord = aTexCoord;
}