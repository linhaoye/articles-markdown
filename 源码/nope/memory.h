#ifndef __MEM_H__
#define __MEM_H__

#define MAP_FILE_SIZE 64
#ifndef MAP_FILE
#define MAP_FILE 0
#endif

struct smem_t;
typedef struct smem_t smem_t;

void *share_memory_new(smem_t *smt, int size, char *file);

int share_memory_delete(smem_t *smt);

#endif