#include "shader.h"
#include <stdio.h>
#include <stdlib.h>

// Helper function to read a file into a string
static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        printf("Failed to open file: %s\n", path);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

bool shader_load(Shader* shader, const char* vertex_path, const char* fragment_path) {
    char* vertex_source = read_file(vertex_path);
    char* fragment_source = read_file(fragment_path);

    if (!vertex_source || !fragment_source) {
        printf("ERROR: Could not find shader files at %s or %s\n", vertex_path, fragment_path);
        return false;
    }

    GLint success;
    char infoLog[512];

    // Compile Vertex
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, (const char**)&vertex_source, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        printf("VERTEX ERROR:\n%s\n", infoLog);
    }

    // Compile Fragment
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, (const char**)&fragment_source, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        printf("FRAGMENT ERROR:\n%s\n", infoLog);
    }

    // Link
    shader->id = glCreateProgram();
    glAttachShader(shader->id, vertex);
    glAttachShader(shader->id, fragment);
    glLinkProgram(shader->id);
    glGetProgramiv(shader->id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader->id, 512, NULL, infoLog);
        printf("LINKING ERROR:\n%s\n", infoLog);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    free(vertex_source);
    free(fragment_source);

    return success; // Returns false if linking failed!
}

void shader_use(Shader* shader) {
    glUseProgram(shader->id);
}

void shader_destroy(Shader* shader) {
    glDeleteProgram(shader->id);
}