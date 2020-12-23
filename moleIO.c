#include "moleIO.h"

ssize_t safe_read(int file_descriptor, char *buf, size_t byte_num){
	ssize_t result;
	ssize_t len = 0;

	do {
		result = TEMP_FAILURE_RETRY(read(file_descriptor, buf, byte_num));
		if(result <= 0) return result;
		buf += result;
		len += result;
		byte_num -= result;
	} while(byte_num > 0);

	return len;
}

ssize_t safe_write(int file_descriptor, char *buf, size_t byte_num){
	ssize_t result;
	ssize_t len = 0;

	do {
		result = TEMP_FAILURE_RETRY(write(file_descriptor, buf, byte_num));
		if(result < 0) return result;
		buf += result;
		len += result;
		byte_num -= result;
	} while(byte_num > 0);

	return len;
}

int is_big_endian(){
	const int temp = 1;
	return (*(char*)&temp == 0);
}

void reverse_bytes(char *bytes, int size_of_var){
	for(int i = 0; i < size_of_var/2; i++){
		char temp = bytes[i];
		bytes[i] = bytes[size_of_var-1-i];
		bytes[size_of_var-1-i] = temp;
	}
}

int read_integer(int file_descriptor, void *integer, int size_of_integer){
	if(size_of_integer <= 0) return 1;

	ssize_t result = safe_read(file_descriptor, integer, size_of_integer);
	if(result != size_of_integer) return 1;

	if(!is_big_endian())
		reverse_bytes((char*)integer, size_of_integer);

	return 0;
}

int write_integer(int file_descriptor, void *integer, int size_of_integer){
	if(size_of_integer <= 0) return 1;

	if(!is_big_endian())
		reverse_bytes((char*)integer, size_of_integer);

	ssize_t result = safe_write(file_descriptor, integer, size_of_integer);
	if(result != size_of_integer) return 1;

	if(!is_big_endian())
		reverse_bytes((char*)integer, size_of_integer);

	return 0;
}

ssize_t read_string(int file_descriptor, char** buf, unsigned int *size, unsigned int max_size){
	unsigned int  string_size;
	if(read_integer(file_descriptor, &string_size, sizeof(unsigned int)))
	   return -1;

	if(string_size > max_size)
		return -1;

	if(size != NULL) *size = string_size;

	*buf = (char*)malloc(sizeof(char)*string_size);
	if(*buf == NULL) return -1;

	ssize_t result = safe_read(file_descriptor, *buf, string_size);
	if(result != (ssize_t)string_size) return -1;

	return result;
}

ssize_t write_string(int file_descriptor, char *buf, unsigned int size){
	if(write_integer(file_descriptor, &size, sizeof(unsigned int)))
		return -1;

	ssize_t result = safe_write(file_descriptor, buf, size);
	if(result != (ssize_t)size) return -1;

	return result;
}

int parse_int(const char *text, int *result, int min, int max){
	char *endPtr;
	long num = strtol(text, &endPtr, 10);
	if(endPtr != NULL && !*endPtr && num <= INT_MAX &&
	   num >= min && num <= max){
		*result = (int)num;
		return 0;
	}
	return 1;
}

int is_valid_path(const char *path_candidate, enum is_valid_path_mode mode){
	struct stat file_stat;
	if(lstat(path_candidate, &file_stat)) return -1;
	else if(mode == IS_VALID_ALL) return 0;
	else if(mode == IS_VALID_DIR && S_ISDIR(file_stat.st_mode)) return 0;
	else if(mode == IS_VALID_LINK && S_ISLNK(file_stat.st_mode)) return 0;
	else if(mode == IS_VALID_REGF && S_ISREG(file_stat.st_mode)) return 0;
	return -1;
}
