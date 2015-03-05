#define _POSIX_C_SOURCE 2

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MB 1024*1024
#define BUF_64MB 64*MB

void* rdrand_thread(void*);
typedef struct {
	int64_t* buffer;
	size_t start;
	size_t end;
} thread_data;

int main(int argc, char* argv[]) {
	size_t bufsize = BUF_64MB;
	size_t cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
	int out_fd = STDOUT_FILENO;
	
	char c;
	while ((c = getopt(argc, argv, "n:b:d:")) != -1 ) {
		switch (c) {
			case 'n':
				//Thread count
				cpu_count = (int)strtol(optarg, NULL, 10);
				break;
			case 'b':
				//Buffer size
				bufsize = (int)strtol(optarg, NULL, 10)*MB;
				break;
			case 'd':				
				//Device
				if (optarg[0] != '-') { 
					out_fd = open(optarg, O_WRONLY);
				}
				break;
		}
	}

	pthread_t threads[cpu_count];

	int problem = 0;

	int64_t* buffer = malloc(bufsize * sizeof(char));
	while (1) {
		size_t thread_size = bufsize / 8 / cpu_count;
		for (size_t i = 0; i < cpu_count; i++) {
			thread_data* args = malloc(sizeof(thread_data));
			args->buffer = buffer;
			args->start = i * thread_size;
			args->end = ((i + 1) * thread_size) - 1;
			pthread_create(&threads[i], NULL, rdrand_thread, args); 
		}

		for (size_t i = 0; i < cpu_count; i++) {
			pthread_join(threads[i], NULL);	
		}

		if (write(out_fd, (char*)buffer, bufsize) != 0) {
			int err = errno;
			if (err == EPIPE || err == ENOSPC) {
				//EPIPE: no problem, reading piped process ended...
				//ENOSPC: no problem, disk is full
				break;
			} else if (err != 0) {
				fprintf(stderr, "Error: %s\n", strerror(err));
				problem = 1;
				break;
			}	
		}
	}

	free(buffer);
	return problem;

}

void* rdrand_thread(void* data) {
	thread_data args = *(thread_data*)data;
	free(data);

	for (size_t i = args.start; i < args.end; i++) {
		int64_t random;
		__asm__ (
			"rdrand %0;"
			: "=r" (random)
		);
		args.buffer[i] = random;
	}
	pthread_exit(NULL);
}
