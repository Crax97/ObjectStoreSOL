#include "signal.h"
#include "server.h"
#include "commons.h"

#include <stdlib.h>
#include <unistd.h>

extern int SERVER_RUNNING;

void* signal_worker(void* args) {
	struct server_info_s* info = (struct server_info_s*)args;

	sigset_t set;
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGUSR1);

	while (info->server_running) {
		sigprocmask(SIG_BLOCK, &set, NULL);
		int sig;
		SC(sigwait(&set, &sig));

		switch(sig) {
			case SIGINT:
			case SIGTERM:
				printf(OS "Initiating shutdown\n");
				info->server_running = 0;
				close(info->server_fd);
				break;
			case SIGUSR1:
				server_print_info(info);
				sigaddset(&set, SIGINT);
				sigaddset(&set, SIGTERM);
				sigaddset(&set, SIGUSR1);
			break;
		}
	}

	pthread_exit(NULL);
}

pthread_t create_signal_thread(struct server_info_s* info) {
	sigset_t set;
	sigfillset(&set);	
	sigprocmask(SIG_BLOCK, &set, NULL);
	pthread_t thread;
	SC(pthread_create(&thread, NULL, signal_worker, info));
	return thread;
}
