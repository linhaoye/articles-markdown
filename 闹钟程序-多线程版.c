#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct alarm_tag {
	int secondes;
	char message[64];
} alarm_t;


void *alarm_thread(void *arg)
{
	alarm_t *alarm = (alarm_t*)arg;
	int status;

	status = pthread_detach(pthread_self());
	if (status != 0) {
		fprintf(stderr, "Detach trhead\n");
		exit(1);
	}
	sleep(alarm->secondes);
	printf("\n(%d %s)\n", alarm->secondes, alarm->message);
	free(alarm);
	return NULL;
}

int main(int argc, char const *argv[])
{
	int status;
	char line[128];
	alarm_t *alarm;
	pthread_t thread;

	while (1) {
		printf("Alarm> ");
		if (fgets(line, sizeof(line), stdin) == NULL) exit(1);
		if (strlen(line) <= 1) continue;
		alarm = malloc(sizeof(alarm_t));

		if (alarm == NULL) {
			printf("Allocate alarm\n");
			exit(1);
		}

		if (sscanf(line, "%d %64[^\n]", &alarm->secondes, alarm->message) < 2) {
			fprintf(stderr, "Bad command\n");
			exit(1);
		} else {
			status = pthread_create(&thread, NULL, alarm_thread, alarm);

			if (status != 0) {
				fprintf(stderr, "Create thread\n");
				exit(1);
			}
		}
	}

	return 0;
}
