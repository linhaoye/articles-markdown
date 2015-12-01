typedef struct
{
	unit_test_func func;
	char *comment;
	int run_times;
	char *key;
} hashmap_unit_test;

static hash_map *utmap = NULL;