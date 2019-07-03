#ifndef SIGNAL_H
#define SIGNAL_H

#include <pthread.h>
#include <signal.h>

struct server_info_s;
pthread_t create_signal_thread(struct server_info_s* info);

#endif
