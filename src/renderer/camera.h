#ifndef CAMERA_H
#define CAMERA_H

#include <cglm.h>
#include <GLFW/glfw3.h>

typedef struct {
    vec3 position;
    vec3 front;
    vec3 up;
    float yaw;
    float pitch;
    float speed;
    float sensitivity;
} Camera;

void camera_init(Camera* cam);
void camera_update_vectors(Camera* cam);
void camera_process_input(Camera* cam, GLFWwindow* window, float deltaTime);
void camera_process_mouse(Camera* cam, float xoffset, float yoffset);

#endif