#include "indexingThread.h"

int set_signal_handler(void (*f)(int), int sig){
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if(sigaction(sig, &act, NULL) == -1) return 1;
	return 0;
}

void user_signal_handler(int sig){
	(void)(sig); // make unused warning not show up
}

void* indexing_thread(void* vargs){
	mole_data_t* data = (mole_data_t*)vargs;

	if(setup_indexing_thread(data)) return NULL;

	while(1){
		struct timespec time_val = (struct timespec){data->indexing_interval+data->last_indexing_time-time(0), 0};

		// sleep for t seconds
		if(time_val.tv_sec > 0 && nanosleep(&time_val, NULL) == -1 && errno != EINTR){
			fprintf(stderr, "nanosleep function failed. Exiting indexing thread.\n");
			return NULL;
		}
		// perform indexing if exit flag is not set
		pthread_mutex_lock(data->exit_flag_mutex);
		if(!data->exit_flag){
			pthread_mutex_unlock(data->exit_flag_mutex);
			if(perform_indexing(data)){
				fprintf(stderr, "Could not index directory. Exiting indexing thread.\n");
				return NULL;
			}
			replace_current_index_tree(data);
		}
		else pthread_mutex_unlock(data->exit_flag_mutex);

		// block SIGUSR1 signal for file I/O operations
		if(pthread_sigmask(SIG_BLOCK, &data->mask, NULL) || pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL)){
			fprintf(stderr, "pthread_sigmask function failed. Exiting indexing thread.\n");
			return NULL;
		}

		if(save_data(data)) return NULL;
		fprintf(stderr, "Indexing finished.\n");

		if(pthread_sigmask(SIG_UNBLOCK, &data->mask, NULL) || pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)){
			fprintf(stderr, "Indexing thread fatal error. Exiting...\n");
			return NULL;
		}

		// check exit flag
		pthread_mutex_lock(data->exit_flag_mutex);
		if(data->exit_flag){
			fprintf(stderr, "Exiting indexing thread...\n");
			pthread_mutex_unlock(data->exit_flag_mutex);
			return NULL;
		}
		else pthread_mutex_unlock(data->exit_flag_mutex);
	}
}

int setup_indexing_thread(mole_data_t* data){
	sigemptyset(&data->mask);
	sigaddset(&data->mask, SIGUSR1);

	// basically ignore SIGUSR1 signal, but enable it to interrupt nanosleep
	if(set_signal_handler(user_signal_handler, SIGUSR1)){
		fprintf(stderr, "sigaction function failed. Exiting indexing thread.\n");
		return 1;
	}

	// block SIGUSR1 signal for file I/O operations
	if(pthread_sigmask(SIG_BLOCK, &data->mask, NULL)){
		fprintf(stderr, "pthread_sigmask function failed. Exiting indexing thread.\n");
		return 1;
	}

	if(load_data(data)) return 1;

	if(pthread_sigmask(SIG_UNBLOCK, &data->mask, NULL)){
		fprintf(stderr, "pthread_sigmask function failed. Exiting indexing thread.\n");
		return 1;
	}

	replace_current_index_tree(data);
	return 0;
}

void replace_current_index_tree(mole_data_t *data){
	index_tree_t *temp, *temp2;
	clone_tree(data->new_index_tree, &temp);
	pthread_mutex_lock(data->current_index_tree_mutex);
	temp2 = data->current_index_tree;
	data->current_index_tree = temp;
	pthread_mutex_unlock(data->current_index_tree_mutex);
	clean_tree(&temp2);
}

int perform_indexing(mole_data_t *data){
	pthread_mutex_lock(data->indexing_flag_mutex);
	data->indexing_flag = 1;
	pthread_mutex_unlock(data->indexing_flag_mutex);
	clean_tree(&data->new_index_tree);
	if(index_directory(&data->new_index_tree, data->path_d, strlen(data->path_d))){
		pthread_mutex_lock(data->indexing_flag_mutex);
		data->indexing_flag = 0;
		pthread_mutex_unlock(data->indexing_flag_mutex);
		return 1;
	}
	pthread_mutex_lock(data->indexing_flag_mutex);
	data->indexing_flag = 0;
	pthread_mutex_unlock(data->indexing_flag_mutex);
	data->last_indexing_time = time(0);
	return 0;
}

int load_data(mole_data_t* data){
	printf("Loading data...\n");

	load_modification_time(data);

	// read data from the index file
	if(load_index_file(data) == 0){
		printf("Index file loaded.\n");
		return 0;
	}

	// if it can't load file, it will try to index the directory
	printf("Indexing...\n");
	if(perform_indexing(data)){
		fprintf(stderr, "Cant index directory specified by path_d argument.\n");
		return 1;
	}

	if(save_data(data)) return 1;
	printf("Indexing finished.\n");

	return 0;
}

void load_modification_time(mole_data_t *data){
	struct stat file_stat;
	if(stat(data->path_f, &file_stat)){
		fprintf(stderr, "Could not read modification time of the index file.\n");
		data->last_indexing_time = 0;
	}
	else{
		data->last_indexing_time = file_stat.st_mtime;
	}
}

int load_index_file(mole_data_t *data){
	int file_descriptor = open(data->path_f, O_RDONLY);
	int failed_flag = 1;

	if(file_descriptor < 0){
		fprintf(stderr, "Could not open the index file.\n");
		return 1;
	}
	else if(read_index_tree(file_descriptor, &data->new_index_tree)){
		fprintf(stderr, "Could not parse the index file.\n");
		clean_tree(&data->new_index_tree);
	}
	else if(strncmp(data->new_index_tree->file_attr.filename, data->path_d, data->new_index_tree->file_attr.filename_size) != 0){
		fprintf(stderr, "Path in the index file doesnt match the patch in path-f argument. Reindexing.\n");
		clean_tree(&data->new_index_tree);
	}
	else failed_flag = 0;

	if(close(file_descriptor)){
		clean_tree(&data->new_index_tree);
		return 1;
	}
	return failed_flag;
}

int save_data(mole_data_t* data){
	int file_descriptor = open(data->path_f, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if(file_descriptor < 0){
		fprintf(stderr, "open function failed. Exiting indexing thread.\n");
		return 1;
	}

	if(write_index_tree(file_descriptor, data->new_index_tree)){
		fprintf(stderr, "Could not save index tree. Exiting indexing thread.\n");
		return 1;
	}

	if(close(file_descriptor)){
		fprintf(stderr, "close function failed. Exiting indexing thread.\n");
		return 1;
	}
	return 0;
}
