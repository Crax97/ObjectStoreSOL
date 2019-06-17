#include "commons.h"
#include "server.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <memory.h>
#include <sys/stat.h>
#include <limits.h>
#include <dirent.h>

#define CHUNK_SIZE 1024 

void realloc_and_append(char** line, int *curpos, char* buf, int off) {
	*line = (char*)realloc(*line, (*curpos + off) * sizeof(char));
	buf[off] = 0;
	memcpy( (*line) + *curpos, buf, off);	
	*curpos += off;
	(*line)[*curpos] = '\0';
}

ssize_t read_to_newline(int fd, char** line) {
	*line = NULL;

	char chunk[CHUNK_SIZE + 1];
	char c = 'a';	

	int chunk_pos = 0, msg_pos = 0;
	while (c != '\n' ) {
		errno = 0;
		int rn = read(fd, &c, 1); 
		if( rn < 0) {
			if (errno == EINTR) {
				continue;
			} else {
				if (*line) free (*line);
				return -1;
			}
		} else if( rn == 0 ) {
			realloc_and_append(line, &msg_pos, chunk, chunk_pos);
			return msg_pos + chunk_pos - 1;;	
		} else {
			chunk[chunk_pos] = c;
			chunk_pos ++;
			if(c == '\n' || chunk_pos == CHUNK_SIZE) {
				realloc_and_append(line, &msg_pos, chunk, chunk_pos);
				chunk_pos = 0;
			}
		}
	}
	return msg_pos;
}

ssize_t read_data(int fd, char* data, size_t len) {
	
	char useless;
	read(fd, &useless, 1);

	size_t total = 0;
	size_t left = len;
	while(total < len) {
		errno = 0;
		int rn = read(fd, data + total, left);
		if(rn < 0) {
			if(errno == EINTR) {
				rn = 0;
			} else {
				return -1;				
			}
		} else if(rn == 0) {
			data[total] = '\0';
			return total;
		} else {
			total += rn;
			left -= rn;	
		}	
	}
	data[total] = '\0';
	return total;
}

ssize_t writen (int fd, const char* buf, size_t len) {
	size_t total = 0;
	size_t left = len;
	while(total < len) {
		errno = 0;
		int now = write(fd, buf + total, left);
		if(now < 0) {
			if(errno == EINTR) {
				now = 0;
			} else {
				return -1;
			}
		} else if (now == 0) {
			return total - 1;
		} 
		total += now;
		left -= now;
	}
	return total;
}

int create_folder(char* path) {
	struct stat info;
	if(stat(path, &info) != 0) {
		return mkdir(path, DEFAULT_MASK) == 0;
	}
	return 1;
}

int delete_file(char* path) {
	return unlink(path) == 0;
}

int read_from_disk(char* path, char** buf) {
	FILE* file = fopen(path, "r");
	if (file != NULL) {
		struct stat info;
		if(stat(path, &info) != 0) {
			return OS_ERR;
		}
	
		size_t len = info.st_size;
		*buf = (char*) calloc(len, sizeof(char));	

		while(len > 0) {
			size_t now = fread(*buf, sizeof(char), len, file);
			len -= now;
		}
		return info.st_size - len;
	} 
return OS_ERR;
}

int write_to_disk(char* path, char* buf, ssize_t len) {
	FILE* file = fopen(path, "w");

	if(file == NULL) {
		return OS_ERR;
	}	

	ssize_t written = 0;
	while(written < len) {
		size_t now = fwrite(buf, sizeof(char), len, file);
		written += now;
	}
	fclose(file);
	return written;
}
void gather_directory_elts_and_size(DIR* dir, size_t* elts, size_t* size) {
	char buf[PATH_MAX];
	getcwd(buf, PATH_MAX);
	printf("scanning dir %s\n", buf); 
	struct dirent* cur_dir = readdir(dir);
	struct stat info;
	if(stat(cur_dir->d_name, &info) == 0) {
		while(cur_dir != NULL) {
			if (strcmp(cur_dir->d_name, ".") == 0 ||
				strcmp(cur_dir->d_name, "..") == 0) {
				//nothing	
			} else {
				struct stat cur_scanned_info;
				char cur_el[PATH_MAX];
				sprintf(cur_el, "%s/%s", buf, cur_dir->d_name);
				if( stat(cur_el, &cur_scanned_info) == 0 && S_ISDIR(cur_scanned_info.st_mode)) {
					DIR* newdir = opendir(cur_dir->d_name);
					if(newdir != NULL) {
						SC(chdir(cur_dir->d_name));
						gather_directory_elts_and_size(newdir, elts, size);
						closedir(newdir);
					}
				} else {
					(*elts)++;
					(*size) += info.st_size;
				}
			}
			cur_dir = readdir(dir);
		}
	} else {
		perror("Could not open directory");
	}
	SC(chdir(".."));

}	

void server_print_info(const struct server_info_s* info) {
	printf("[Object Store] Server infos:\n");
	size_t total_dim = 0, total_count = 0;
	DIR* data_dir = opendir("data");
	if(data_dir != NULL) {
		chdir("data");
		gather_directory_elts_and_size(data_dir, &total_count, &total_dim);
		closedir(data_dir);
		printf("[Object Store] Clients connected: %lu\n", info->active_clients);
		printf("[Object Store] Total size: %lu\n", total_dim);
		printf("[Object Store] Total elements: %lu\n", total_count);	
	} else {
		fprintf(stderr, "[Object Store] could not open the data directory!");
		exit(EXIT_FAILURE);
	}
}
