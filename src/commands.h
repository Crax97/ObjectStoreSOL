#ifndef COMMANDS_H
#define COMMANDS_H

struct worker_s;
struct server_info_s;

int handle_register(char* name, struct worker_s* worker, struct server_info_s* info); 
int handle_store(char* data_name, char* data_len, struct worker_s* worker);
int handle_retrieve(char* data_name, struct worker_s* worker);
int handle_delete(char* data_name, struct worker_s* worker);
int handle_leave(struct worker_s* worker, struct server_info_s* info);

int ko(char* msg, struct worker_s* worker);
int ok(struct worker_s* worker);

#endif
