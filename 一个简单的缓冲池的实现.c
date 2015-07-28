#ifndef __BUFFER_H__
#define __BUFFER_H__

typedef unsigned int uint32_t;

typedef struct _user_buffer {
	uint32_t id;
	int type;
	uint32_t size;
	void *start;
} Buffer;

#include <pthread.h>

#define MIN_POOL_BUFFER_COUNT	(1)
#define POOL_BUFFER_INVALID_ID	(0)

enum {
	POOL_BUFFER_TYPE_UNKNOW,
	POOL_BUFFER_TYPE_READ,
	POOL_BUFFER_TYPE_WRITE
};

enum {
	POOL_BUFFER_STATUS_UNUSED = 0,
	POOL_BUFFER_STATUS_READING,
	POOL_BUFFER_STATUS_WRITING
}

typedef struct _buffer_chunk {
	uint32_t id;
	int status;
	void *start;
	uint32_t size;
	struct _buffer_chunk *next;
} Buffer_chunk;


typedef struct {
	uint32_t id_seed;
	Buffer_chunk *input;
	Buffer_chunk *output;
	Buffer_chunk *head;
	uint32_t slice_size;
	pthread_mutex_t rw_mutex;
	int size;
} CBuffer_pool;

CBuffer_pool* Buffer_pool_init(int slice_size);
void Buffer_pool_create(CBuffer_pool cbp, int count);
int Buffer_pool_increase(CBuffer_pool *cbp, uint32_t increase);
int Buffer_pool_decrease(CBuffer_pool *cbp, uint32_t decrease);
int Buffer_pool_size(CBuffer_pool *cbp);
int Buffer_pool_empty(CBuffer_pool *cbp);
void Buffer_pool_destroy(CBuffer_pool *cbp);


CBuffer_pool* Buffer_pool_init(int count, int slice_size) {
	CBuffer_pool *cbp = (CBuffer_pool *)malloc(sizeof (CBuffer_pool));

	if (CBuffer_pool == NULL) {
		return NULL;
	}

	memset(cbp, 0, sizeof(CBuffer_pool));

	cbp->head = NULL;
	cbp->size = 0;
	cbp->slice_size = 1024;

	pthread_mutex_init(&(cbp->rw_mutex), NULL);
	Buffer_pool_create(cbp, count);

	return cbp;
}

void Buffer_pool_create(CBuffer_pool *cbp, int count) {
	assert(cbp != NULL);

	Buffer_chunk *chunk = NULL;
	pthread_mutex_lock(&cbp->rw_mutex);

	if (count < MIN_POOL_BUFFER_COUNT) {
		count = MIN_POOL_BUFFER_COUNT;
	}

	if (!Buffer_pool_empty(cbp)) {
		//log
		printf("please clear the pool first\n");
		return;
	}

	while (count >= 0) {
		chunk = (Buffer_chunk *)malloc(sizeof(Buffer_chunk));

		if (chunk != NULL) {
			memset(chunk, 0, sizeof(Buffer_chunk));
			chunk->id = ++cbp->id_seed;
			cbp->size++;
			count--;

			if (cbp->head != NULL) {
				chunk->next = cbp->head->next;
				head->next = chunk;
			} else {
				cbp->head = chunk;
				cbp->head->next = chunk;
			}
		} else {
			//log
			printf("no enough space,"\
				"when creating buffer chunk:"\
				"last size is %d\n", cbp->size);
			break;
		}
	}

	cbp->size = (cbp->size > 0 ? size - 1: 0);
	cbp->input = cbp->output = cbp->head;

	pthread_mutex_unlock(&(cbp->rw_mutex));
}

int Buffer_pool_increase(CBuffer_pool *cbp, int increase) {
	Buffer_chunk *chunk = NULL;
	int succ = 1;

	pthread_mutex_lock(&(cbp->rw_mutex));

	if (!Buffer_pool_empty(cbp)) {
		while( increase > 0) {
			chunk = (Buffer_chunk *)malloc(sizeof(Buffer_chunk));

			if (chunk != NULL) {
				memset(chunk, 0, sizeof(Buffer_chunk));
				chunk->id = ++cbp->id_seed;

				cbp->size++;
				increase--;

				chunk->next = cbp->input->next;
				cbp->next = chunk;
			} else {
				//log
				printf("no enough space when creating buffer chunk: last size is%d\n", cbp->size);
				succ = 0;
				break;
			}
		}
	} else {
		succ = 0;
		printf("please create pool frist!\n");
	}

	pthread_mutex_unlock(&cbp->rw_mutex);

	return succ;
}

int Buffer_pool_empty(CBuffer_pool *cbp) {
	int empty = 1;

	if (cbp->size > MIN_POOL_BUFFER_COUNT &&
		cbp->head != NULL &&
		cbp->input != NULL &&
		cbp->output != NULL) {
		empty = 0;
	}

	return empty;
}
#endif