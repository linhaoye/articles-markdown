#include <stdio.h>
#include <stdlib.h>

#include <event.h>

int main(void)
{

	event_init();

	/*printf("base info: evsel[%p], evbase[%p], evcount[%d], evacount[%d], activequeue[%p], evqueue[%p]\n",
			base->evsel,
			base->evbase,
			base->event_count,
			base->event_count_active,
			base->activequeues,
			base->eventquque);*/

	struct event evfifo;
	event_set(&evfifo, 0, EVENT_READ, NULL, &evfifo);
	event_add(&evfifo, NULL);

	return 0;
}
