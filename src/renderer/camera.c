#include "camera.h"

void camera_init(Camera* cam) {
    glm_vec3_copy((vec3){8.0f, 8.0f, -5.0f}, cam->position);
    glm_vec3_copy((vec3){0.0f, -0.5f, -1.0f}, cam->front);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, cam->up);
    cam->yaw = 90.0f;
    cam->pitch = -20.0f;
    cam->speed = 10.0f;
    cam->sensitivity = 0.1f;
}

void camera_process_input(Camera* cam, GLFWwindow* window, float deltaTime) {
    float velocity = cam->speed * deltaTime;
    vec3 dir;

    // FORWARD (W)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm_vec3_scale(cam->front, velocity, dir);
        glm_vec3_add(cam->position, dir, cam->position);
    }
    // BACKWARD (S)
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_scale(cam->front, velocity, dir);
        glm_vec3_sub(cam->position, dir, cam->position);
    }
    // STRAFE LEFT (A)
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        vec3 right;
        glm_vec3_cross(cam->front, cam->up, right); // Calculate Right vector
        glm_vec3_normalize(right);
        glm_vec3_scale(right, velocity, dir);
        glm_vec3_sub(cam->position, dir, cam->position); // Move opposite of Right
    }
    // STRAFE RIGHT (D)
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        vec3 right;
        glm_vec3_cross(cam->front, cam->up, right);
        glm_vec3_normalize(right);
        glm_vec3_scale(right, velocity, dir);
        glm_vec3_add(cam->position, dir, cam->position); // Move along Right
    }
}

void camera_process_mouse(Camera* cam, float xoffset, float yoffset) {
    xoffset *= cam->sensitivity;
    yoffset *= cam->sensitivity;

    cam->yaw   += xoffset;
    cam->pitch += yoffset;

    // Constraint: Begrenzen (Limit) the pitch so the screen doesn't flip
    if (cam->pitch > 89.0f)  cam->pitch = 89.0f;
    if (cam->pitch < -89.0f) cam->pitch = -89.0f;

    camera_update_vectors(cam);
}

void camera_update_vectors(Camera* cam) {
    vec3 direction;
    // The "Magic" math to turn angles into a 3D direction
    direction[0] = cos(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
    direction[1] = sin(glm_rad(cam->pitch));
    direction[2] = sin(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
    
    glm_vec3_normalize_to(direction, cam->front);
}