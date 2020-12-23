#ifndef INDEXTREE_H
#define INDEXTREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH_SIZE 4096
#define MAX_FILENAME_SIZE 255
#define FILE_TYPE_NUM 6

#include "list.h"
#include "moleIO.h"
#include "streamHandler.h"

// resolve circular dependency
typedef struct search_output_stream search_output_stream_t;

typedef enum file_type {
	FILE_TYPE_DIR,
	FILE_TYPE_JPG,
	FILE_TYPE_PNG,
	FILE_TYPE_GZIP,
	FILE_TYPE_ZIP,
	FILE_TYPE_OTHER
} file_type_t;

typedef struct file_attributes {
	char* filename;
	unsigned int filename_size;
	off_t size;
	uid_t uid;
	file_type_t type;
} file_attributes_t;

// structure that stores search options
// it's useful in search_tree function
typedef struct search_options {
	int print;

	int counter[FILE_TYPE_NUM];
	int counter_opt;

	off_t larger_than;
	int larger_than_opt;

	char* name_part;
	unsigned int name_part_size;

	uid_t uid;
	int uid_opt;
} search_options_t;

// structure storing indexed directory tree
// index_tree can represent a directory or a regular file
// in case of a directory p_children will store its
// subdirectories and files
typedef struct index_tree {
	list_t children;
	unsigned int children_num;
	file_attributes_t file_attr;
} index_tree_t;

// loads and parses structure from a file using an open file descriptor
// returns 0 if it was successful or 1 if it failed
// it allocates memory!
int read_file_attributes(int file_descriptor, file_attributes_t* p_file_attr);
int read_index_tree(int file_descriptor, index_tree_t **pp_tree);

// saves structure to a file using an open file descriptor
// returns 0 if it was successful or 1 if it failed
int write_file_attributes(int file_descriptor, file_attributes_t* p_file_attr);
int write_index_tree(int file_descriptor, index_tree_t* p_tree);

// initializes index_tree_t structure
void init_tree(index_tree_t *tree);

// deallocates memory
void clean_tree(index_tree_t **tree);

// clones a tree (deepclone)
int clone_tree(index_tree_t *tree, index_tree_t** clone);

// traverses the index_tree_t structures and runs
// search_tree_check_conditions on each file entry
void search_tree(char* path, unsigned int path_len, index_tree_t* tree, search_options_t* search_options, search_output_stream_t* out);

// checks if file satisfies the conditions and prints it out using search_output_stream_t structure
void search_tree_check_conditions(char* path, index_tree_t* tree, search_options_t* search_options, search_output_stream_t *out);

#endif
