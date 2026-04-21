#ifndef SHADER_H
#define SHADER_H

#include <glad.h>
#include <stdbool.h>

typedef struct {
    GLuint id;
} Shader;

// Functions to load, use, and destroy the shader
bool shader_load(Shader* shader, const char* vertex_path, const char* fragment_path);
void shader_use(Shader* shader);
void shader_destroy(Shader* shader);

#endif