#include "indexer.h"

int index_directory(index_tree_t** pp_tree, char path[MAX_PATH_SIZE], unsigned int path_len){
	// allocate memory for root and start read_directory function on it
	*pp_tree = (index_tree_t*)malloc(sizeof(index_tree_t));
	init_tree(*pp_tree);
	if(*pp_tree == NULL) return 1;
	if(read_file_info(path, path_len, 0, &(*pp_tree)->file_attr)){
		return 1;
	}
	if(read_directory(path, path_len, *pp_tree)){
		return 1;
	}
	return 0;
}

int read_directory(char* path, unsigned int path_len, index_tree_t* directory){
	// open directory and read file system entries
	// add them to the children list of current node

	directory->children_num = 0;
	directory->children = NULL;
	list_t child_p = directory->children;

	DIR* dir = opendir(path);
	struct dirent* df;
	if(dir == NULL){
		return 1;
	}
	pthread_cleanup_push(cleanup_dir, (void*)dir);

	do{
		errno = 0;
		df = readdir(dir);
		if(df != NULL && !(!strncmp(df->d_name, "..", MAX_FILENAME_SIZE) || !strncmp(df->d_name, ".", MAX_FILENAME_SIZE))){
			unsigned int filename_size = strnlen(df->d_name, MAX_FILENAME_SIZE);

			if(list_push(&child_p)) return 1;
			directory->children_num++;

			if(directory->children_num == 1)
				directory->children = child_p;

			child_p->val = (index_tree_t*)malloc(sizeof(index_tree_t));
			init_tree(child_p->val);
			if(child_p->val == NULL) return 1;

			index_tree_t* tree = (index_tree_t*)child_p->val;

			tree->children = NULL;
			int snprintf_ret = snprintf(path+path_len, MAX_PATH_SIZE-path_len, "/%s", df->d_name);
			if(snprintf_ret == MAX_PATH_SIZE - (int)path_len - 1)
				fprintf(stderr, "Warning: Path string may be truncated. Try to increase MAX_PATH_SIZE.");
			path[path_len+filename_size+1]=0;
			if(read_file_info(path, path_len+filename_size+1, path_len+1, &tree->file_attr)){
				fprintf(stderr, "Can't access [%s]. Skipping.\n", path);
				clean_tree(&tree);
				list_pop(&child_p);
				if(directory->children_num == 0)
					directory->children = NULL;
				directory->children_num--;
			}
			else if(tree->file_attr.type == FILE_TYPE_OTHER){
				clean_tree(&tree);
				list_pop(&child_p);
				directory->children_num--;
				if(directory->children_num == 0)
					directory->children = NULL;
			}
		}

	} while(df != NULL);
	pthread_cleanup_pop(1);
	if(errno != 0) return 1;

	// loop through the children list of current node
	// and run read_directory function for every file system entry
	// from the list that is a directory

	child_p = directory->children;
	while(child_p != NULL){
		index_tree_t* tree = child_p->val;
		if(tree->file_attr.type == FILE_TYPE_DIR){
			int snprintf_ret = snprintf(path+path_len, MAX_PATH_SIZE-path_len, "/%s", tree->file_attr.filename);
			if(snprintf_ret == MAX_PATH_SIZE - (int)path_len - 1)
				fprintf(stderr, "Warning: Path string may be truncated. Try to increase MAX_PATH_SIZE.");
			path[path_len+tree->file_attr.filename_size]=0;
			if(read_directory(path, path_len+tree->file_attr.filename_size, tree)){
				fprintf(stderr, "Can't access %s. Skipping.\n", path);
			}
			path[path_len] = 0;
		}
		child_p = child_p->next;
	}
	return 0;
}

void cleanup_dir(void* dir){
	closedir(dir);
}

int read_file_info(char* path, size_t path_len, size_t filename_index, file_attributes_t* file_attr){
	struct stat file_stat;
	file_attr->filename = NULL;
	if(lstat(path, &file_stat)) return 1;

	file_attr->size = file_stat.st_size;
	file_attr->uid = file_stat.st_uid;
	if(S_ISDIR(file_stat.st_mode)){
		file_attr->type = FILE_TYPE_DIR;
	}
	else if(file_attr->size < MAX_MAGIC_NUMBERS_LEN || !S_ISREG(file_stat.st_mode)){
		file_attr->type = FILE_TYPE_OTHER;
		return 0;
	}
	else if(get_file_type(path, &file_attr->type)) return 1;

	file_attr->filename = (char*)malloc(sizeof(char)*(path_len-filename_index+1));
	if(file_attr->filename == NULL) return 1;
	strncpy(file_attr->filename, path+filename_index, path_len-filename_index);
	file_attr->filename[path_len-filename_index] = 0;
	file_attr->filename_size = path_len-filename_index+1;
	return 0;
}

int get_file_type(char* path, enum file_type* type){
	*type = FILE_TYPE_OTHER;
	int file_descriptor = open(path, O_RDONLY);
	if(file_descriptor < 0){
		char buff[MAX_PATH_SIZE];
		getcwd(buff, MAX_PATH_SIZE);
	}
	char buf[MAX_MAGIC_NUMBERS_LEN];
	if(safe_read(file_descriptor, buf, MAX_MAGIC_NUMBERS_LEN) != MAX_MAGIC_NUMBERS_LEN)
		return 1;
	if(close(file_descriptor)) return 1;

	if(buf[0] == '\x50' && buf[1] == '\x4B' && buf[2] == '\x03' && buf[3] == '\x04')
		*type = FILE_TYPE_ZIP;
	else if(buf[0] == '\x1F' && buf[1] == '\x8B' && buf[2] == '\x08')
		*type = FILE_TYPE_GZIP;
	else if(buf[0] == '\x89' && buf[1] == '\x50' && buf[2] == '\x4E' && buf[3] == '\x47')
		*type = FILE_TYPE_PNG;
	else if(buf[0] == '\xFF' && buf[1] == '\xD8')
		*type = FILE_TYPE_JPG;

	return 0;
}
