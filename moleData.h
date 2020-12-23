#ifndef MOLE_DATA_H
#define MOLE_DATA_H

#include <time.h>
#include <pthread.h>

#include "indexTree.h"

// a structure that stores the most important program informations
// and has access to almost every dynamically allocated variable

typedef struct mole_data {
	char path_d[MAX_PATH_SIZE];
	char path_f[MAX_PATH_SIZE];

	index_tree_t *current_index_tree;
	pthread_mutex_t *current_index_tree_mutex;

	index_tree_t *new_index_tree;

	int indexing_interval;
	time_t last_indexing_time;

	pthread_t indexing_thread_tid;

	int exit_flag;
	pthread_mutex_t *exit_flag_mutex;

	int indexing_flag;
	pthread_mutex_t *indexing_flag_mutex;

	sigset_t mask;
} mole_data_t;

int init_mole_data(mole_data_t *data);
void clean_mole_data(mole_data_t *data);

#endif
