#include "terminal.h"

void run_terminal(mole_data_t* data){
	while(1){
		if(pthread_mutex_trylock(data->current_index_tree_mutex) || data->current_index_tree == NULL || data->current_index_tree->file_attr.filename == NULL){
			printf("[]>");
		}
		else{
			printf("[%s]>", data->current_index_tree->file_attr.filename);
			pthread_mutex_unlock(data->current_index_tree_mutex);
		}
		pthread_mutex_unlock(data->current_index_tree_mutex);

		char* cmd_buf = NULL;
		size_t cmd_len = 0;
		if(getline(&cmd_buf, &cmd_len, stdin) < 0){
			free(cmd_buf);
			return;
		}
		if(parse_and_run_command(data, cmd_buf, cmd_len)){
			free(cmd_buf);
			return;
		}
		free(cmd_buf);
	}
}

int parse_args(int argc, char** argv, mole_data_t* data){
	// help message
	if(argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "help") == 0)){
		show_help_message();
		return 1;
	}

	opterr = 0;
	int arg;

	data->path_d[0] = 0;
	data->path_f[0] = 0;
	// small cheat
	// default value for indexing interval is INT_MAX
	// in reality that means indexing will never be done without
	// "index" command
	data->indexing_interval = INT_MAX;

	while((arg = getopt(argc, argv, "d:f:t:")) != -1){
		switch(arg){
		case 'd':
			if(parse_path_d(data, optarg))
				return 1;
			break;
		case 'f':
			if(parse_path_f(data, optarg))
				return 1;
			break;
		case 't':
			if(parse_int(optarg, &data->indexing_interval, MIN_T_ARG, MAX_T_ARG) != 0){
				fprintf(stderr, "t argument is not a valid integer or is not within bounds\n");
				return 1;
			}
			break;
		default:
			break;
		}
	}
	if(optind < argc){
		printf("Non-option arguments were specified\n");
		return 1;
	}
	if(data->path_d[0] == 0){ // path_d was not set
		if(parse_path_d(data, NULL))
			return 1;
	}
	if(data->path_f[0] == 0){ // path_d was not set
		if(parse_path_f(data, NULL))
			return 1;
	}
	fix_path(data->path_d);
	fix_path(data->path_f);
	return 0;
}

void fix_path(char* path){
	int index = strnlen(path, MAX_PATH_SIZE);
	if(path[index-1] == '/') path[index-1] = 0;
}

int parse_path_d(mole_data_t* data, char* optarg){
	if(optarg != NULL){ // parse path_d from an argument
		if(data->path_d[0] != 0){
			fprintf(stderr, "path_d was specified twice\n");
			return 1;
		}
		else if(strnlen(optarg, MAX_PATH_SIZE) == MAX_PATH_SIZE){
			fprintf(stderr, "path_d argument is too long\n");
			return 1;
		}
		else if(is_valid_path(optarg, IS_VALID_DIR) != 0){
			fprintf(stderr, "path_d is not a valid directory\n");
			return 1;
		}
		else{
			char* temp = realpath(optarg, NULL);
			if(temp == NULL) return 1;
			int snprintf_ret = snprintf(data->path_d, MAX_PATH_SIZE, "%s", temp);
			if(snprintf_ret == MAX_PATH_SIZE - 1)
				fprintf(stderr, "Warning: Path string may be truncated. Try to increase MAX_PATH_SIZE.");
			free(temp);
			return 0;
		}
	}
	else return set_path_d_default(data);
}

int parse_path_f(mole_data_t* data, char* optarg){
	if(optarg != NULL){ // parse path_f from an argument
		if(data->path_f[0] != 0){
			fprintf(stderr, "path_f was specified twice\n");
			return 1;
		}
		else if(strnlen(optarg, MAX_PATH_SIZE) == MAX_PATH_SIZE){
			fprintf(stderr, "path_f argument is too long\n");
			return 1;
		}
		else{ // extract dirname, get full path to parent directory, join with filename
			char* filename = basename(optarg);
			if(filename == NULL) return 1;
			char* dirname_temp = dirname(optarg);
			char* real_dirname = realpath(dirname_temp, NULL);
			int snprintf_ret = snprintf(data->path_f, MAX_PATH_SIZE, "%s/%s", real_dirname, filename);
			if(snprintf_ret == MAX_PATH_SIZE - 1)
				fprintf(stderr, "Warning: Path string may be truncated. Try to increase MAX_PATH_SIZE.");
			free(real_dirname);
			if(!is_valid_path(data->path_f, IS_VALID_DIR)){
				fprintf(stderr, "path_f argument is a path to a directory\n");
				return 1;
			}
			return 0;
		}
	}
	else return set_path_f_default(data);
}

int set_path_d_default(mole_data_t* data){
	char* env_path_d = getenv("MOLE_DIR");
	if(env_path_d != NULL && strnlen(env_path_d, MAX_PATH_SIZE) != MAX_PATH_SIZE){
		int snprintf_ret = snprintf(data->path_d, MAX_PATH_SIZE, "%s", env_path_d);
		if(snprintf_ret == MAX_PATH_SIZE - 1)
			fprintf(stderr, "Warning: Path string may be truncated. Try to increase MAX_PATH_SIZE.");
		return 0;
	}
	else {
		fprintf(stderr, "path_d was not specified and it is not set in enviromental variables\n");
		return 1;
	}
}

int set_path_f_default(mole_data_t* data){
	char* env_path_f = getenv("MOLE_INDEX_PATH");
	if(env_path_f != NULL && strnlen(env_path_f, MAX_PATH_SIZE) != MAX_PATH_SIZE){
		int snprintf_ret = snprintf(data->path_f, MAX_PATH_SIZE, "%s", env_path_f);
		if(!is_valid_path(data->path_f, IS_VALID_DIR)){
			fprintf(stderr, "MOLE_INDEX_PATH is is a path to a directory\n");
			return 1;
		}
		if(snprintf_ret == MAX_PATH_SIZE - 1)
			fprintf(stderr, "Warning: Path string may be truncated. Try to increase MAX_PATH_SIZE.");
		return 0;
	}
	else {
		char* home_path = getenv("HOME");
		if(home_path != NULL && strnlen(home_path, MAX_PATH_SIZE) != MAX_PATH_SIZE){
			int snprintf_ret = snprintf(data->path_f, MAX_PATH_SIZE, "%s/%s", home_path, MOLE_DEFAULT_FILE);
			if(snprintf_ret == MAX_PATH_SIZE - 1)
				fprintf(stderr, "Warning: Path string may be truncated. Try to increase MAX_PATH_SIZE.");
			return 0;
		}
		else {
			fprintf(stderr, "Couldn't set path_f to default value\n");
			return 1;
		}
	}
}

command_t terminal_commands[] = {
	{"index", &index_command },
	{"exit", &exit_command },
	{"exit!", &exit_now_command},
	{"owner", &owner_command},
	{"largerthan", &largerthan_command},
	{"namepart", &namepart_command},
	{"count", &count_command},
	{"help", &help_command}
};

int parse_and_run_command(mole_data_t* data, char* cmd_buf, unsigned int cmd_len){
	char *p = cmd_buf;
	char *word = cmd_buf;

	search_options_t search_options;
	memset(&search_options, 0, sizeof(search_options_t));
	int parse_status = 0;
	unsigned int commands_arr_size = sizeof(terminal_commands)/sizeof(command_t);

	while(!next_word(&p, &word)){
		for(unsigned int i = 0; i < commands_arr_size; i++){
			if(strncmp(word, terminal_commands[i].command_text, cmd_len) == 0){
				int ret_val = (terminal_commands[i].command_func)(&p, &word, &parse_status, data, &search_options);
				if(ret_val) return 1;
				break;
			}
		}
		if(parse_status == 0)
			parse_status = -1;
		if(parse_status == -1) break;
	}

	if(parse_status == -1){
		printf("Unknown command. Type \"help\" to show help message.\n");
		return 0;
	}
	else if(parse_status == 1){
		perform_search(data, search_options);
	}
	return 0;
}

void perform_search(mole_data_t *data, search_options_t search_options){
	pthread_mutex_lock(data->current_index_tree_mutex);

	if(search_options.counter_opt) search_options.print = 0;

	if(data->current_index_tree == NULL){
		fprintf(stderr, "The directory is not yet indexed\n");
	}
	else {
		char path[MAX_PATH_SIZE];
		search_output_stream_t out;
		init_search_output_stream(&out);
		int snprintf_ret = snprintf(path, MAX_PATH_SIZE, "%s", data->current_index_tree->file_attr.filename);
		if(snprintf_ret == MAX_PATH_SIZE - 1)
			fprintf(stderr, "Warning: Path string may be truncated. Try to increase MAX_PATH_SIZE.");
		search_tree(path, data->current_index_tree->file_attr.filename_size, data->current_index_tree, &search_options, &out);
		destroy_search_output_stream(&out);
		if(search_options.counter_opt)
			search_options.print = 0;
	}
	pthread_mutex_unlock(data->current_index_tree_mutex);

	if(search_options.counter_opt){
		printf("-----------------\n");
		printf("|DIR |%10d|\n", search_options.counter[FILE_TYPE_DIR]);
		printf("|JPG |%10d|\n", search_options.counter[FILE_TYPE_JPG]);
		printf("|PNG |%10d|\n", search_options.counter[FILE_TYPE_PNG]);
		printf("|ZIP |%10d|\n", search_options.counter[FILE_TYPE_ZIP]);
		printf("|GZIP|%10d|\n", search_options.counter[FILE_TYPE_GZIP]);
		printf("-----------------\n");
	}
}

int next_word(char** p, char** word){
	if(*(*p+1) == 0) return 1;
	while(**p == 0 || **p == ' ' || **p == '\t') (*p)++;
	*word = *p;
	while(**p != '\n' && **p != ' ' && **p != '\t') (*p)++;
	**p = 0;
	return 0;
}

int next_word_quotation(char **p, char** word){
	if(*(*p+1) == 0) return 1;
	while(**p == 0 || **p == ' ' || **p == '\t') (*p)++;
	if(**p != '\"') return 1;
	(*p)++;
	*word = *p;
	while(**p != '\n' &&
		  !(**p == '\"' && ( (*(*p-1) != '\\') || ( (*(*p-1) == '\\') && (*(*p-2) == '\\')))))
		(*p)++;
	if(**p == '\n') return 1;
	**p = 0;
	(*p)++;
	**p = 0;
	return 0;
}

void delete_escaping_sequences(char* p){
	int offset = 0;
	while(*p != 0){
		if(*p == '\\'){
			offset++;
			p++;
		}
		*(p-offset) = *p;
		if(*p == 0) return;
		p++;
	}
	*(p-offset) = *p;
	return;
}

int index_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options){
	(void) p; (void)word; (void)parse_status; (void)search_options;
	if(*parse_status != 0){
		*parse_status = -1;
		return 0;
	}
	else {
		pthread_mutex_lock(data->indexing_flag_mutex);
		if(data->indexing_flag){
			pthread_mutex_unlock(data->indexing_flag_mutex);
			printf("Indexing has already started.\n");
		}
		else{
			pthread_mutex_unlock(data->indexing_flag_mutex);
			pthread_kill(data->indexing_thread_tid, SIGUSR1);
		}
		*parse_status = 1;
		return 0;
	}
}

int exit_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options){
	(void)p; (void)word; (void)parse_status, (void)search_options;
	if(*parse_status != 0){
		*parse_status = -1;
		return 0;
	}
	pthread_mutex_lock(data->exit_flag_mutex);
	data->exit_flag = 1;
	pthread_mutex_unlock(data->exit_flag_mutex);
	pthread_kill(data->indexing_thread_tid, SIGUSR1);
	void* return_value;
	if(pthread_join(data->indexing_thread_tid, &return_value)){
		return 1;
	}
	clean_mole_data(data);
	return 1;
}

int exit_now_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options){
	(void)p; (void)word; (void)parse_status; (void)search_options;
	if(*parse_status != 0){
		*parse_status = -1;
		return 0;
	}
	pthread_cancel(data->indexing_thread_tid);
	void* return_value;
	if(pthread_join(data->indexing_thread_tid, &return_value)){
		return 1;
	}
	clean_mole_data(data);
	return 1;
}

int owner_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options){
	(void)data;
	int arg_int;
	if(next_word(p, word) ||
	   parse_int(*word, &arg_int, 0, INT_MAX)){
		*parse_status = -1;
		return 0;
	}
	search_options->uid = (uid_t)arg_int;
	search_options->uid_opt = 1;
	search_options->print = 1;
	*parse_status = 1;
	return 0;
}

int largerthan_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options){
	(void)data;
	int arg_int;
	if(next_word(p, word) ||
	   parse_int(*word, &arg_int, 0, INT_MAX)){
		*parse_status = -1;
		return 0;
	}
	search_options->larger_than = (size_t)arg_int;
	search_options->larger_than_opt = 1;
	search_options->print = 1;
	*parse_status = 1;
	return 0;
}

int namepart_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options){
	(void)data;
	if(next_word_quotation(p, word)){
		printf("namepart parsing failed\n");
		*parse_status = -1;
		return 0;
	}
	search_options->name_part = *word;
	search_options->name_part_size = strlen(*word);
	search_options->print = 1;
	*parse_status = 1;
	return 0;
}

int count_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options){
	(void)p; (void)word; (void)parse_status; (void)data;
	search_options->counter_opt = 1;
	*parse_status = 1;
	return 0;
}

int help_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options){
	(void)p; (void)word; (void)data; (void)search_options;
	*parse_status = 2;
	show_help_message();
	return 0;
}

void show_help_message(){
	printf("----HELP----\nMole - a small program for indexing and searching through directories, JPG, PNG, GZIP and ZIP files.\n");
	printf("It loads filesystem tree to a structure in memory storing some informations about files.\n");
	printf("It also saves and loads (if it's possible) informations about indexed directories to/from a file,\n");
	printf("which allows to quickly resumy querying a big directory with many subdirectories.\n");
	printf("Full indexing process can take several minutes, because the type of a file is detected from\n");
	printf("a magic number, not from a filename extension, so the process is done in the background allowing\n");
	printf("you to execute commands on previous data.\n\n");
	printf("Parameters:\n\n    -d path_d   -    specify the directory you want to index\n");
	printf("if path_d is not specified, the program will try to read path from an enviromental variable MOLE_DIR.\n\n");
	printf("    -f path_f   -    specify the file you to save to/load from information about the directory.\n");
	printf("if path_f is not specified, the program will try to read path from an enviromental variable MOLE_INDEX_FILE.\n");
	printf("If it is also not set, the program will try to load ~/.mole-index\n\n");
    printf("    -t <SEC>    -    if t argument is specified, then every t seconds the program will perform indexing of\n");
	printf("path_d directory in the background updating informations for your queries. <SEC> value must be an integer between 30\n");
	printf("and 7200.\n\n");
	printf("Commands: \n");
	printf("help             -  shows this message\n");
	printf("count            -  shows a table with the number of files of every file type\n");
	printf("largerthan <X>   -  shows files larger than <X> bytes\n");
	printf("owner <X>        -  shows files with uid equal to <X>\n");
	printf("namepart \"<X>\"   -  shows files whose name contains text <X>\n");
	printf("example: namepart \".jpg\"\n");
	printf("Type \\\" to escape \\ sign\n");
	printf("Type \\\\ to escape \\\n");
	printf("NOTE: you can combine last 4 command for example \"count owner 1000 largerthan 500000\"\n");
	printf("index            -  starts new indexing in the background\n");
	printf("exit             -  waits for indexing to end and exits the program.\n");
	printf("exit!            -  interrupts indexing and exits the program.\n------------\n");
}
