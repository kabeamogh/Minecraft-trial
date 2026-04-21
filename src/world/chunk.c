#include "chunk.h"
#include <glad.h>
#include <stdlib.h>
#include <string.h>

// Tile IDs (Assuming a 16x16 texture atlas)
// 0 = Grass Top, 1 = Grass Side, 2 = Dirt, 3 = Stone

static int get_texture_id(uint8_t block_id, int face_dir) {
    // check_directions order: 0=Front, 1=Back, 2=Left, 3=Right, 4=Top, 5=Bottom
    
    if (block_id == 1) { // GRASS BLOCK
        if (face_dir == 4) return 0; // Top: Grass Top
        if (face_dir == 5) return 2; // Bottom: Dirt
        return 1; // All sides: Grass Side
    }
    if (block_id == 2) return 2; // DIRT: All sides dirt
    if (block_id == 3) return 3; // STONE: All sides stone
    
    return 0; // Default fallback
}

// 1. Neighbor offsets for Culling
static int check_directions[6][3] = {
    { 0,  0,  1}, // Front (z+)
    { 0,  0, -1}, // Back  (z-)
    {-1,  0,  0}, // Left  (x-)
    { 1,  0,  0}, // Right (x+)
    { 0,  1,  0}, // Top   (y+)
    { 0, -1,  0}  // Bottom(y-)
};

// 2. Face Templates
static const float face_positions[6][18] = {
    { 0,0,1, 1,0,1, 1,1,1, 1,1,1, 0,1,1, 0,0,1 }, // Front
    { 1,0,0, 0,0,0, 0,1,0, 0,1,0, 1,1,0, 1,0,0 }, // Back
    { 0,0,0, 0,0,1, 0,1,1, 0,1,1, 0,1,0, 0,0,0 }, // Left
    { 1,0,1, 1,0,0, 1,1,0, 1,1,0, 1,1,1, 1,0,1 }, // Right
    { 0,1,1, 1,1,1, 1,1,0, 1,1,0, 0,1,0, 0,1,1 }, // Top
    { 0,0,0, 1,0,0, 1,0,1, 1,0,1, 0,0,1, 0,0,0 }  // Bottom
};

static const float face_uvs[12] = {
    0,0, 1,0, 1,1, 1,1, 0,1, 0,0
};

// Internal helper to get block data
static uint8_t get_block(Chunk* chunk, int x, int y, int z) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) 
        return 0; // Return Air if outside chunk boundaries
    return chunk->blocks[x + CHUNK_SIZE * (y + CHUNK_SIZE * z)];
}

void chunk_init(Chunk* chunk, int x, int y, int z) {
    chunk->x = x; chunk->y = y; chunk->z = z;
    chunk->vertex_count = 0;
    memset(chunk->blocks, 0, sizeof(chunk->blocks));
}

void chunk_generate_flat(Chunk* chunk) {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int y = 0; y < CHUNK_SIZE; y++) {
                int index = x + CHUNK_SIZE * (y + CHUNK_SIZE * z);
                
                if (y == 3) {
                    chunk->blocks[index] = 1; // Grass layer on top
                } else if (y < 3 && y > 0) {
                    chunk->blocks[index] = 2; // Dirt underneath
                } else if (y == 0) {
                    chunk->blocks[index] = 3; // Stone at the very bottom
                } else {
                    chunk->blocks[index] = 0; // Air
                }
            }
        }
    }
}

void chunk_mesh_generate(Chunk* chunk) {
    // Max faces: 16*16*16 blocks * 6 faces * 6 vertices * 5 floats (Pos+UV)
    float* mesh_data = malloc(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 180 * sizeof(float));
    int v_idx = 0;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                uint8_t block = get_block(chunk, x, y, z);
                if (block == 0) continue; 

                for (int d = 0; d < 6; d++) {
                    // Culling logic: only add face if neighbor is air
                    if (get_block(chunk, x + check_directions[d][0], 
                                  y + check_directions[d][1], 
                                  z + check_directions[d][2]) == 0) {
                        
                        // 1. Get the texture ID based on block type and face direction
                        int tex_id = get_texture_id(block, d);
                        
                        // 2. Calculate Atlas math (assuming 16x16 grid)
                        float atlas_size = 16.0f;
                        float tile_size = 1.0f / atlas_size;
                        
                        float col = (float)(tex_id % 16);
                        float row = (float)(tex_id / 16);
                        
                        float u_offset = col * tile_size;
                        float v_offset = 1.0f - ((row + 1.0f) * tile_size); // Inverted row for OpenGL

                        // 3. Calculate GLOBAL world position for this block
                        float world_x = (chunk->x * CHUNK_SIZE) + x;
                        float world_y = (chunk->y * CHUNK_SIZE) + y;
                        float world_z = (chunk->z * CHUNK_SIZE) + z;

                        for (int v = 0; v < 6; v++) {
                            // Position - Use world_x/y/z instead of just x/y/z!
                            mesh_data[v_idx++] = world_x + face_positions[d][v*3 + 0];
                            mesh_data[v_idx++] = world_y + face_positions[d][v*3 + 1];
                            mesh_data[v_idx++] = world_z + face_positions[d][v*3 + 2];
                            
                            // Base UVs
                            float base_u = face_uvs[v*2 + 0];
                            float base_v = face_uvs[v*2 + 1];
                            
                            // Atlas-adjusted UVs
                            mesh_data[v_idx++] = u_offset + (base_u * tile_size);
                            mesh_data[v_idx++] = v_offset + (base_v * tile_size);
                        }
                    }
                }
            }
        }
    }

    chunk->vertex_count = v_idx / 5;

    glGenVertexArrays(1, &chunk->vao);
    glGenBuffers(1, &chunk->vbo);

    glBindVertexArray(chunk->vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
    glBufferData(GL_ARRAY_BUFFER, v_idx * sizeof(float), mesh_data, GL_STATIC_DRAW);

    // Pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // UV
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    free(mesh_data); // Crucial: Beseitigen (Eliminate) the CPU memory after GPU upload
}

void chunk_set_block(Chunk* chunk, int x, int y, int z, uint8_t block_id) {
    // 1. Ensure we don't write outside the array boundaries
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        return; 
    }
    
    // 2. Calculate the 1D array index
    int index = x + CHUNK_SIZE * (y + CHUNK_SIZE * z);
    
    // 3. Update the data
    chunk->blocks[index] = block_id;
    
    // 4. Rebuild the mesh to show the changes visually
    chunk_mesh_generate(chunk);
}