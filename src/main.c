#include <glad.h>
#include <GLFW/glfw3.h>
#include <cglm.h>
#include <stdio.h>

#include "gfx/window.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/camera.h"
#include "world/chunk.h"

void update_world(Chunk* chunks, int center_x, int center_z) {
    int i = 0;
    for (int cx = center_x - 1; cx <= center_x + 1; cx++) {
        for (int cz = center_z - 1; cz <= center_z + 1; cz++) {
            // Clean up old memory to avoid a crash or memory leak
            glDeleteBuffers(1, &chunks[i].vbo);
            glDeleteVertexArrays(1, &chunks[i].vao);

            // Re-initialize the chunk at its new world position
            chunk_init(&chunks[i], cx, 0, cz);
            chunk_generate_flat(&chunks[i]);
            chunk_mesh_generate(&chunks[i]);
            i++;
        }
    }
}

int main() {
    printf("--- PROGRAM START ---\n");
    getchar(); // Wait for you to press Enter
    // 1. Initialize Window
    Window window;
    if (window_init(&window, 800, 600, "Minecraft C - Chunk Render") != 0) return -1;

    // 2. Configure OpenGL Global State
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); // Optimization: Don't draw back-faces of triangles
    glfwSetInputMode(window.handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 3. Load Resources
    Shader shader;
    if (!shader_load(&shader, "assets/shaders/default.vert", "assets/shaders/default.frag")) {
        fprintf(stderr, "Could not load shaders!\n");
        return -1;
    }
    unsigned int grassTexture = texture_load("assets/textures/atlas.png");
    unsigned int atlasTexture = texture_load("assets/textures/atlas.png");

    // 4. Initialize Camera
    Camera cam;
    camera_init(&cam);
    float lastFrame = 0.0f;

    // 5. Initialize and Mesh the 3x3 Chunk Grid
    Chunk chunks[9];
    int chunk_count = 0;
    
    // Loop X from -1 to 1, and Z from -1 to 1
    for (int cx = -1; cx <= 1; cx++) {
        for (int cz = -1; cz <= 1; cz++) {
            chunk_init(&chunks[chunk_count], cx, 0, cz);
            chunk_generate_flat(&chunks[chunk_count]);
            chunk_mesh_generate(&chunks[chunk_count]);
            chunk_count++;
        }
    }

    // --- CROSSHAIR SETUP ---
    Shader crosshairShader;
    if (!shader_load(&crosshairShader, "assets/shaders/ui.vert", "assets/shaders/ui.frag")) {
        printf("ERROR: Failed to load UI shaders!\n");
        return -1;
    }

    // 4 Vertices: 2 for the horizontal line, 2 for the vertical line
    float crosshair_verts[] = {
        -0.03f,  0.00f,   0.03f,  0.00f, // Horizontal line (Left to Right)
         0.00f, -0.04f,   0.00f,  0.04f  // Vertical line (Bottom to Top)
    };

    unsigned int crossVAO, crossVBO;
    glGenVertexArrays(1, &crossVAO);
    glGenBuffers(1, &crossVBO);

    glBindVertexArray(crossVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crossVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair_verts), crosshair_verts, GL_STATIC_DRAW);

    // Tell OpenGL how to read the 2D coordinates (size 2 instead of 3)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    int player_current_cX = 0;
    int player_current_cZ = 0;
    update_world(chunks, player_current_cX, player_current_cZ);

    // 6. Main Loop
    while (!window_should_close(&window)) {
        if (glfwGetKey(window.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window.handle, true);
        }
        // Time Logic
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        camera_process_input(&cam, window.handle, deltaTime);
        
        // Mouse look logic (as we implemented before)
        static float lastX = 400, lastY = 300;
        static bool firstMouse = true;
        double xposIn, yposIn;
        glfwGetCursorPos(window.handle, &xposIn, &yposIn);
        if (firstMouse) { lastX = xposIn; lastY = yposIn; firstMouse = false; }
        camera_process_mouse(&cam, xposIn - lastX, lastY - yposIn);
        lastX = xposIn; lastY = yposIn;

        // Rendering
        glClearColor(0.5f, 0.8f, 0.9f, 1.0f); // Sky Blue
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_use(&shader);

        // Matrices
        mat4 model, view, projection;
        glm_mat4_identity(model);
        
        vec3 lookTarget;
        glm_vec3_add(cam.position, cam.front, lookTarget);
        glm_lookat(cam.position, lookTarget, cam.up, view);
        
        // Get current window size to handle resizing
        int curWidth, curHeight;
        glfwGetFramebufferSize(window.handle, &curWidth, &curHeight);
        glViewport(0, 0, curWidth, curHeight); // Tell OpenGL the new size

        // Recalculate aspect ratio
        float aspect = (float)curWidth / (float)curHeight;
        glm_perspective(glm_rad(45.0f), aspect, 0.1f, 1000.0f, projection);

        int new_cX = (int)floor(cam.position[0] / CHUNK_SIZE);
        int new_cZ = (int)floor(cam.position[2] / CHUNK_SIZE);
    
        if (new_cX != player_current_cX || new_cZ != player_current_cZ) {
            player_current_cX = new_cX;
            player_current_cZ = new_cZ;
            update_world(chunks, player_current_cX, player_current_cZ);
        }

        // --- RAYCASTING (Block Breaking & Placing in Grid) ---
        static bool left_mouse_was_pressed = false;
        static bool right_mouse_was_pressed = false;
        
        bool left_mouse_is_pressed = glfwGetMouseButton(window.handle, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bool right_mouse_is_pressed = glfwGetMouseButton(window.handle, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

        // Run raycast if EITHER button was just pressed
        if ((left_mouse_is_pressed && !left_mouse_was_pressed) || 
            (right_mouse_is_pressed && !right_mouse_was_pressed)) {
            
            vec3 ray_pos;
            glm_vec3_copy(cam.position, ray_pos);
            
            float step_size = 0.05f; // Move 0.05 units per check
            float max_distance = 5.0f; // Reach of the player
            
            vec3 step_vector;
            glm_vec3_scale(cam.front, step_size, step_vector);

            // Variables to remember the last empty air block (GLOBAL coordinates)
            int prev_bX = -1, prev_bY = -1, prev_bZ = -1; 

            for (float t = 0; t < max_distance; t += step_size) {
                // 1. Get exact GLOBAL block coordinates
                int bX = (int)floor(ray_pos[0]);
                int bY = (int)floor(ray_pos[1]);
                int bZ = (int)floor(ray_pos[2]);

                // 2. Determine which Chunk these coordinates belong to
                int target_cX = (int)floor((float)bX / CHUNK_SIZE);
                int target_cZ = (int)floor((float)bZ / CHUNK_SIZE);
                
                // 3. Convert Global to Local Chunk Coordinates (0 to 15)
                int local_X = bX - (target_cX * CHUNK_SIZE);
                int local_Y = bY; 
                int local_Z = bZ - (target_cZ * CHUNK_SIZE);

                // 4. Find the matching chunk in our 9-chunk array
                Chunk* hit_chunk = NULL;
                for(int i = 0; i < 9; i++) {
                    if (chunks[i].x == target_cX && chunks[i].z == target_cZ) {
                        hit_chunk = &chunks[i];
                        break;
                    }
                }

                // 5. If we found a valid chunk and Y is within bounds, check for a hit
                if (hit_chunk != NULL && local_Y >= 0 && local_Y < CHUNK_SIZE) {
                    int index = local_X + CHUNK_SIZE * (local_Y + CHUNK_SIZE * local_Z);
                    
                    if (hit_chunk->blocks[index] != 0) { // HIT! Solid Block.
                        
                        if (left_mouse_is_pressed) {
                            // Destroy the block
                            chunk_set_block(hit_chunk, local_X, local_Y, local_Z, 0); 
                        } 
                        else if (right_mouse_is_pressed) {
                            // Build a block at the PREVIOUS air position
                            if (prev_bY >= 0 && prev_bY < CHUNK_SIZE) {
                                // Calculate chunk & local coordinates for the previous block
                                int p_cX = (int)floor((float)prev_bX / CHUNK_SIZE);
                                int p_cZ = (int)floor((float)prev_bZ / CHUNK_SIZE);
                                int p_local_X = prev_bX - (p_cX * CHUNK_SIZE);
                                int p_local_Z = prev_bZ - (p_cZ * CHUNK_SIZE);
                                
                                // Find the specific chunk that holds the previous air block
                                for(int i = 0; i < 9; i++) {
                                    if (chunks[i].x == p_cX && chunks[i].z == p_cZ) {
                                        // Insert a stone block (ID 3)
                                        chunk_set_block(&chunks[i], p_local_X, prev_bY, p_local_Z, 3);
                                        break;
                                    }
                                }
                            }
                        }
                        break; // Stop the ray
                    }
                }
                
                // Save the previous global coordinates for building
                prev_bX = bX; prev_bY = bY; prev_bZ = bZ;
                
                // Move the ray forward for the next step
                glm_vec3_add(ray_pos, step_vector, ray_pos); 
            }
        }
        
        // Update button states for the next frame
        left_mouse_was_pressed = left_mouse_is_pressed;
        right_mouse_was_pressed = right_mouse_is_pressed;

        // Send to Shader
        glUniformMatrix4fv(glGetUniformLocation(shader.id, "model"), 1, GL_FALSE, (float*)model);
        glUniformMatrix4fv(glGetUniformLocation(shader.id, "view"), 1, GL_FALSE, (float*)view);
        glUniformMatrix4fv(glGetUniformLocation(shader.id, "projection"), 1, GL_FALSE, (float*)projection);

        // Draw the Chunk
        // Draw ALL Chunks
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, atlasTexture);
        shader_use(&shader);
        
        for (int i = 0; i < 9; i++) {
            glBindVertexArray(chunks[i].vao);
            glDrawArrays(GL_TRIANGLES, 0, chunks[i].vertex_count);
        }

        // --- DRAW UI LAST ---
        // Disable Depth Test so the crosshair is never hidden behind a block
        glDisable(GL_DEPTH_TEST); 
        
        shader_use(&crosshairShader);
        glBindVertexArray(crossVAO);
        
        // Draw 4 vertices as 2 individual lines
        glDrawArrays(GL_LINES, 0, 4); 
        
        // Re-enable Depth Test for the next frame's 3D rendering
        glEnable(GL_DEPTH_TEST);

        window_update(&window);
    }

    window_terminate(&window);
    return 0;
}