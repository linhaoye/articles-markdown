#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	int status;
	char line[128];
	int seconds;
	pid_t pid;
	char message[64];

	while (1) {
		printf("Alarm> ");

		if (fgets(line, sizeof(line), stdin) == NULL) exit(1);
		if (sscanf(line, "%d %64[^\n]", &seconds, message) < 2) {
			fprintf(stderr, "Bad Command\n");
		} else {
			pid = fork();

			if (pid == (pid_t)-1) {
				fprintf(stderr, "Fork error\n");
				exit(0);
			} 
			if (pid == (pid_t)0)
			{
				sleep(seconds);
				printf("(%d) %s\n\n", seconds, message);	
				exit(0);
			} else {
				do {
					pid = waitpid((pid_t-1), NULL, WNOHANG);
					if (pid == (pid_t)-1) {
						fprintf(stderr, "Wait for child\n");
						exit(1);
					}
				}while(pid != (pid_t)0)
			}
		}
	}	

	return 0;
}
