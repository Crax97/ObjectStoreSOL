#ifndef SIGNAL_H
#define SIGNAL_H

#include <pthread.h>
#include <signal.h>

static int SIGUSR1_SIGNAL_EMITTED;

struct server_info_s;
pthread_t create_signal_thread(struct server_info_s* info);

#endif
