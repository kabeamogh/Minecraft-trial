#include "window.h"
#include <stdio.h>

void framebuffer_size_callback(GLFWwindow* handle, int width, int height) {
    // Tell OpenGL the new dimensions of the rendering area
    glViewport(0, 0, width, height);
}

int window_init(Window* window, int width, int height, const char* title) {
    printf("[DEBUG] Starting GLFW Init...\n");
    if (!glfwInit()) {
        printf("[ERROR] glfwInit() failed!\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    printf("[DEBUG] Creating Window...\n");
    window->handle = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window->handle) {
        printf("[ERROR] glfwCreateWindow() failed! Check OpenGL version support.\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window->handle);
    glfwSetFramebufferSizeCallback(window->handle, framebuffer_size_callback);

    printf("[DEBUG] Loading GLAD...\n");
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("[ERROR] gladLoadGLLoader() failed!\n");
        return -1;
    }

    window->width = width;
    window->height = height;

    printf("[DEBUG] Window Init Successful!\n");
    return 0;
}

bool window_should_close(Window* window) {
    return glfwWindowShouldClose(window->handle);
}

void window_update(Window* window) {
    glfwSwapBuffers(window->handle);
    glfwPollEvents();
}

void window_terminate(Window* window) {
    glfwDestroyWindow(window->handle);
    glfwTerminate();
}
