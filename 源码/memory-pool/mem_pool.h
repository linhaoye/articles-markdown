#include <sys/types.h>

typedef struct _mcMemoryPool
{
	/**
	 * 指向内在池首地址
	 */
	void *object;

	void* (*alloc)(struct _mcMemoryPool *, uint32_t);
	void* (*free)(struct _mcMemoryPool *, void*);
	void* (*destroy)(struct _mcMemoryPool *);

}  mcMemoryPool;

typedef struct _mcFixedPool_slice
{
	/**
	 * 标记是否占用(0空闲, 1占用)
	 */
	uint8_t lock;

	struct _mcFixedPool_slice *next;
	struct _mcFixedPool_slice *pre;

#ifndef _WIN32
	char data[0];
#else
	char *data;
#endif

} mcFixedPool_slice;

typedef struct _mcFixedPool
{
	/**
	 * 内存指针, 指向一片内存空间
	 */
	void *memory;

	/**
	 * 内存空间大小
	 */
	size_t size;

	mcFixedPool_slice *head;
	mcFixedPool_slice *tail;

	/**
	 * total memory size, 节点数目
	 */
	uint32_t slice_num;

	/**
	 * memory usage, 已经使用的节点
	 */
	uint32_t slice_use;

	 /**
	  * Fixed slice size, not include the memory used by mcFixedPool_slice
	  */
	uint32_t slice_size;

	/**
	 * use shared memory, 是否共享内存
	 */
	uint8_t shared;

} mcFixedPool;

mcMemoryPool* mcFixedPool_new(uint32_t slice_num, uint32_t slice_size, uint8_t shared);
mcMemoryPool* mcFixedPool_new2(uint32_t slice_size, void *memory, size_t size);
