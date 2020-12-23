#include "moleData.h"

int init_mole_data(mole_data_t *data){
	data->exit_flag = 0;
	data->indexing_flag = 0;

	data->current_index_tree_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if(data->current_index_tree_mutex == NULL) return 1;
	if(pthread_mutex_init(data->current_index_tree_mutex, NULL)) return 1;

	data->exit_flag_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if(data->exit_flag_mutex == NULL) return 1;
	if(pthread_mutex_init(data->exit_flag_mutex, NULL)) return 1;

	data->indexing_flag_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if(data->indexing_flag_mutex == NULL) return 1;
	if(pthread_mutex_init(data->indexing_flag_mutex, NULL)) return 1;

	data->new_index_tree = NULL;
	data->current_index_tree = NULL;

	data->indexing_thread_tid = 0;

	data->last_indexing_time = 0;

	return 0;
}

void clean_mole_data(mole_data_t *data){
	clean_tree(&data->current_index_tree);
	clean_tree(&data->new_index_tree);
	free(data->current_index_tree_mutex);
	free(data->exit_flag_mutex);
	free(data->indexing_flag_mutex);
}
