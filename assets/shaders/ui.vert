#version 330 core
layout (location = 0) in vec2 aPos; // Only 2D (X, Y)

void main() {
    // We pass the 2D coordinates directly to the screen. Z is 0 (flat), W is 1.
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}