#ifndef WINDOW_H
#define WINDOW_H

#include <glad.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef struct {
    GLFWwindow* handle;
    int width;
    int height;
} Window;

// The updated return type
int window_init(Window* window, int width, int height, const char* title);

// The functions the Linker was crying about:
bool window_should_close(Window* window);
void window_update(Window* window);
void window_terminate(Window* window);

#endif