#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>

typedef struct worker {
	void *(*process) (void *arg);
	void *arg;
	struct worker *next;
} CThread_workder;

typedef struct {
	pthread_mutex_t queue_lock;
	pthread_cond_t queue_ready;
	CThread_workder *queue_head;
	int shutdown;
	pthread_t *thread_id;
	int max_thread_num;
	int cur_queue_size;
} CThread_pool;

CThread_pool *Pool_init(int max_thread_num);
int Pool_add_worker (CThread_pool *pool, void *(*process)(void *arg), void *arg );
int Pool_destroy(CThread_pool *pool);
void *Pool_thread_routine(void *arg);

CThread_pool *Pool_init(int max_thread_num) {
	CThread_pool *pool = (CThread_pool*)malloc(sizeof (CThread_pool));
	
	if (pool == NULL ) {
		return NULL;
	}

	pthread_mutex_init(&(pool->queue_lock), NULL);
	pthread_cond_init(&(pool->queue_ready), NULL);

	pool->queue_head = NULL;
	pool->max_thread_num = max_thread_num;
	pool->cur_queue_size = 0;
	pool->shutdown = 0;
	pool->thread_id = (pthread_t *) malloc (max_thread_num * sizeof (pthread_t));

	int i = 0;

	for (i = 0; i < max_thread_num; i++) {
		pthread_create(&(pool->thread_id[i]), NULL, Pool_thread_routine, pool);
	}
	
	return pool;
}

int Pool_add_worker(CThread_pool *pool, void *(*process)(void *arg), void *arg) {
	CThread_workder *new_worker = (CThread_workder *)malloc(sizeof(CThread_workder));

	new_worker->process = process;
	new_worker->arg = arg;
	new_worker->next = NULL;

	pthread_mutex_lock(&(pool->queue_lock));

	CThread_workder *item = pool->queue_head;

	if (item != NULL){
		while (item->next != NULL) {
			item = item->next;
		}
		item->next = new_worker;
	} else {
		pool->queue_head = new_worker;
	}

	assert(pool->queue_head != NULL);

	pool->cur_queue_size++;
	pthread_mutex_unlock(&(pool->queue_lock));
	pthread_cond_signal(&(pool->queue_ready));

	return 0;
}

int Pool_destroy(CThread_pool *pool) {
	if (pool->shutdown) {
		return -1;
	}
	pool->shutdown = 1;

	pthread_cond_broadcast(&(pool->queue_ready));

	int i;
	for (i = 0; i < pool->max_thread_num; i++) {
		pthread_join(pool->thread_id[i], NULL);
		printf("release the thread worker 0x%x\n", pool->thread_id[i]);
	}
	free(pool->thread_id);

	CThread_workder *head = NULL;

	while (pool->queue_head != NULL) {
		head = pool->queue_head;
		pool->queue_head = pool->queue_head->next;
		free(head);
	}

	pthread_mutex_destroy(&(pool->queue_lock));
	pthread_cond_destroy(&(pool->queue_ready));

	free(pool);

	return 0;
}

void *Pool_thread_routine(void *arg) {
	printf("starting thread 0x%x\n", pthread_self());

	CThread_pool *pool = (CThread_pool *)arg;

	while(1) {
		pthread_mutex_lock(&(pool->queue_lock));

		while(pool->cur_queue_size == 0 && !pool->shutdown) {
			printf("thread 0x%x is waiting\n", pthread_self());
			pthread_cond_wait(&(pool->queue_ready), &(pool->queue_lock));
		}

		if (pool->shutdown) {
			pthread_mutex_unlock(&(pool->queue_lock));
			printf("thread 0x%x will exit!\n", pthread_self());
			pthread_exit(NULL);
		}

		printf("thread 0x%x is starting to work\n", pthread_self());

		assert(pool->cur_queue_size != 0);
		assert(pool->queue_head != NULL);

		pool->cur_queue_size--;

		CThread_workder *worker = pool->queue_head;
		pool->queue_head = worker->next;
		pthread_mutex_unlock(&(pool->queue_lock));
		
		worker->process(worker->arg); // run process
		free(worker);
		worker = NULL;
	}
	pthread_exit(NULL);
}

void *my_process(void *arg) {
	printf("thread_id is 0x%x, working on task %d\n", pthread_self(), *(int*)arg);
	sleep(1);
	return NULL;
}

main(void)
{
	CThread_pool *pool = Pool_init(3);

	int workingnum[10];
	int i;

	for (i = 0; i < 10; i++) {
		workingnum[i] = i * 2;
		Pool_add_worker(pool, my_process, &workingnum[i]);
	}

	sleep(5);

	printf("\nbegin to release sources\n");
	Pool_destroy(pool);
	printf("\nend");
}
