#include "indexTree.h"

int read_file_attributes(int file_descriptor, file_attributes_t* p_file_attr){
	if(read_string(file_descriptor, &p_file_attr->filename, &p_file_attr->filename_size, MAX_FILENAME_SIZE) < 0) return 1;
	if(read_integer(file_descriptor, &p_file_attr->size, sizeof(off_t))) return 1;
	if(read_integer(file_descriptor, &p_file_attr->uid, sizeof(uid_t))) return 1;
	if(read_integer(file_descriptor, &p_file_attr->type, sizeof(file_type_t))) return 1;
	return 0;
}

int read_index_tree(int file_descriptor, index_tree_t **pp_tree){
	*pp_tree = (index_tree_t*)malloc(sizeof(index_tree_t));
	init_tree(*pp_tree);

	if(read_file_attributes(file_descriptor, &(*pp_tree)->file_attr)){
		return 1;
	}
	if(read_integer(file_descriptor, &(*pp_tree)->children_num, sizeof(unsigned int))){
		return 1;
	}

	list_t p = (*pp_tree)->children;
	for(size_t i = 0; i < (*pp_tree)->children_num; i++){
		list_push(&p);
		if(read_index_tree(file_descriptor, (index_tree_t**)&p->val)){
			list_clear(&p);
			list_clear(&(*pp_tree)->children);
			return 1;
		}
		else if((*pp_tree)->children == NULL){
			(*pp_tree)->children = p;
		}
	}

	return 0;
}

int write_file_attributes(int file_descriptor, file_attributes_t* p_file_attr){
	if(write_string(file_descriptor, p_file_attr->filename, p_file_attr->filename_size) < 0) return 1;
	if(write_integer(file_descriptor, &p_file_attr->size, sizeof(off_t))) return 1;
	if(write_integer(file_descriptor, &p_file_attr->uid, sizeof(uid_t))) return 1;
	if(write_integer(file_descriptor, &p_file_attr->type, sizeof(file_type_t))) return 1;
	return 0;
}

int write_index_tree(int file_descriptor, index_tree_t* p_tree){
	if(write_file_attributes(file_descriptor, &p_tree->file_attr))
		return 1;


	if(write_integer(file_descriptor, &p_tree->children_num, sizeof(unsigned int)))
		return 1;

	list_t p = p_tree->children;
	for(size_t i = 0; i < p_tree->children_num; i++){
		write_index_tree(file_descriptor, p->val);
		p = p->next;
	}

	return 0;
}

void init_tree(index_tree_t *tree){
	tree->children = NULL;
	tree->children_num = 0;
	tree->file_attr.filename = NULL;
}

void clean_tree(index_tree_t **pp_tree){
	if(pp_tree == NULL) return;
	if(*pp_tree == NULL) return;

	if((*pp_tree)->file_attr.filename != NULL){
		free((*pp_tree)->file_attr.filename);
	}

	list_t p = (*pp_tree)->children;
	while(p != NULL){
		clean_tree((index_tree_t**)&p->val);
		list_t temp = p->next;
		free(p);
		p = temp;
	}
	free(*pp_tree);
	*pp_tree = NULL;
}

int clone_tree(index_tree_t *tree, index_tree_t** clone){
	*clone = (index_tree_t*)malloc(sizeof(index_tree_t));
	if(*clone == NULL) return 1;
	init_tree(*clone);

	(*clone)->file_attr = tree->file_attr;
	(*clone)->file_attr.filename = (char*)malloc(sizeof(char)*(*clone)->file_attr.filename_size);
	if((*clone)->file_attr.filename == NULL) return 1;
	strncpy((*clone)->file_attr.filename, tree->file_attr.filename, (*clone)->file_attr.filename_size);

	list_t p = tree->children;
	list_t p2 = NULL;
	while(p != NULL){
		if(list_push(&p2)) return 1;
		if(clone_tree(p->val, (index_tree_t**)&p2->val)) return 1;
		if((*clone)->children == NULL) (*clone)->children = p2;
		p = p->next;
	}
	(*clone)->children_num = tree->children_num;
	return 0;
}

void search_tree(char* path, unsigned int path_len, index_tree_t* tree, search_options_t* search_options, search_output_stream_t* out){
	list_t p = tree->children;
	while(p != NULL){
		index_tree_t* child = p->val;
		int snprintf_ret = snprintf(path+path_len-1, MAX_PATH_SIZE-path_len, "/%s", child->file_attr.filename);
		if(snprintf_ret == MAX_PATH_SIZE - (int)path_len - 1)
			fprintf(stderr, "Warning: Path string may be truncated. Try to increase MAX_PATH_SIZE.");
		search_tree_check_conditions(path, p->val, search_options, out);
		search_tree(path, child->file_attr.filename_size+path_len, child, search_options, out);
		p = p->next;
	}
}

void search_tree_check_conditions(char* path, index_tree_t* tree, search_options_t* search_options, search_output_stream_t *out){
	int flag = 1;
	if(search_options->larger_than_opt && tree->file_attr.size <= search_options->larger_than)
		flag = 0;
	if(search_options->uid_opt && tree->file_attr.uid != search_options->uid)
		flag = 0;
	if(flag && search_options->name_part != NULL){
		int flag2 = 0;
		char* filename = tree->file_attr.filename;
		unsigned int filename_size = tree->file_attr.filename_size;
		char* name_part = search_options->name_part;
		unsigned int name_part_size = search_options->name_part_size;
		if(filename_size >= name_part_size){
			for(unsigned int i = 0; i <= filename_size-name_part_size; i++){
				if(memcmp(filename+i, name_part, name_part_size) == 0){
					flag2 = 1;
					break;
				}
			}
		}
		if(flag2 == 0) flag = 0;
	}
	if(flag && search_options->counter_opt){
		search_options->counter[tree->file_attr.type]++;
	}
	if(search_options->print && flag){
		fprintf(out->stream, "%s [%lu bytes] ", path, tree->file_attr.size);
		if(tree->file_attr.type == FILE_TYPE_DIR) fprintf(out->stream, "[DIR]\n");
		else if(tree->file_attr.type == FILE_TYPE_JPG) fprintf(out->stream, "[JPG]\n");
		else if(tree->file_attr.type == FILE_TYPE_PNG) fprintf(out->stream, "[PNG]\n");
		else if(tree->file_attr.type == FILE_TYPE_GZIP) fprintf(out->stream, "[GZIP]\n");
		else if(tree->file_attr.type == FILE_TYPE_ZIP) fprintf(out->stream, "[ZIP]\n");
		else fprintf(out->stream, "\n");
		update_search_output_stream(out);
	}
}
