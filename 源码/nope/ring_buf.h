#ifndef __RING_BUF__
#define __RING_BUF__

struct ringbuf_t;
typedef struct ringbuf_t ringbuf_t;

ringbuf_t *ringbuf_new(size_t length);
int ringbuf_read(ringbuf_t *buf, void *data, size_t sz);
int ringbuf_write(ringbuf_t *buf, void *data, size_t sz);
int ringbuf_empty(ringbuf_t *buf);
int ringbuf_full(ringbuf_t *buf);
void *ringbuf_get(ringbuf_t *buf, int amount);


#endif
