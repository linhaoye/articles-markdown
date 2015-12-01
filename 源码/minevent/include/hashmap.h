#ifndef __HASHMAP_H__
#define __HASHMAP_H__

typedef void (*hash_map_dtor)(void *data);

typedef struct
{
	struct hashmap_node *root;
	struct hashmap_node *iterator;
	hash_map_dtor dtor;
} hashmap;

hashmap * hashmap_new(uint32_t bucket_num, hash_map_dtor dtor);
void hashmap_free(hashmap *hmap);

int hashmap_add (hashmap *hmap, char *key, uint16_t key_len, void *data, hash_map_dtor dtor);
void hashmap_add_int (hashmap *hmap, uint64_t key, void *data, hash_map_dtor dtor);
void* hashmap_find (hashmap *hmap, char *key, uint16_t key_len);
void* hashmap_find_int (hashmap *hmap, uint64_t key);
void hashmap_update_int (hashmap *hmap, uint64_t key, void *data);
int hashmap_update (hashmap *hmap, char *key, uint16_t key_len, void *data);
int hashmap_del (hashmap *hmap, char *key, uint16_t key_len);
void hashmap_del_int (hashmap *hmap, uint64_t key);
void* hashmap_each (hashmap *hmap, char **key);
void* hashmap_each_int (hashmap *hmap, uint64_t *key);

#define hashmap_each_reset(hmap)	(hmap->iterator = NULL)

#endif
