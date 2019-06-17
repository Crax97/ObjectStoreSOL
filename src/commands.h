#ifndef COMMANDS_H
#define COMMANDS_H

struct worker_s;

int handle_register(char* name, struct worker_s* worker); 
int handle_store(char* data_name, char* data_len, struct worker_s* worker);
int handle_retrieve(char* data_name, struct worker_s* worker);
int handle_delete(char* data_name, struct worker_s* worker);
int handle_leave(struct worker_s* worker);

int ko(char* msg, struct worker_s* worker);
int ok(struct worker_s* worker);

#endif
