#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

#define CHUNK_SIZE 16

typedef struct {
    uint8_t blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    int x, y, z; 
    unsigned int vao, vbo;
    int vertex_count;
} Chunk;

void chunk_init(Chunk* chunk, int x, int y, int z);
void chunk_generate_flat(Chunk* chunk);
// ADD THIS LINE BELOW:
void chunk_mesh_generate(Chunk* chunk); 
void chunk_set_block(Chunk* chunk, int x, int y, int z, uint8_t block_id);

#endif