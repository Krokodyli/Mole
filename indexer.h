#ifndef INDEXER_H
#define INDEXER_H

#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <dirent.h>

#include "indexTree.h"

#define MAX_MAGIC_NUMBERS_LEN 4

// creates model of filesystem tree
// returns 0 if it was successful, 1 otherwise
// every dynamically allocated variable is accessible from the outside of the function
// via index_tree_t structure
// this allows to deallocate memory in case of thread cancelling
int index_directory(index_tree_t** pp_tree, char path[MAX_PATH_SIZE], unsigned int path_len);

// reads file system entries and places them on the children list of the directory
// after that it runs read_directory for every file system entry that is a directory
int read_directory(char *path, unsigned int path_len, index_tree_t *directory);

void cleanup_dir(void* dir);

// fills file_attributes_t structure with the file information
int read_file_info(char *path, size_t path_len, size_t filename_index, file_attributes_t *file_attr);

// uses magic number to determine file format
int get_file_type(char *path, enum file_type *type);

#endif
