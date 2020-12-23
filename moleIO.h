#ifndef MOLE_IO
#define MOLE_IO

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>


// read function that retries if interrupted
ssize_t safe_read(int file_descriptor, char* buf, size_t byte_num);

// write function that retries if interrupted
ssize_t safe_write(int file_descriptor, char* buf, size_t byte_num);

// return 1 if platform is using big-endian
int is_big_endian();

// reverses bytes in the variable
void reverse_bytes(char* bytes, int size_of_var);

// reads 8*size_of_integer bit integer from a file using an open file descriptor
// returns 0 if it was successful or 1 if it failed
int read_integer(int file_descriptor, void* integer, int size_of_integer);

// writes 8*size_of_integer bit integer to a file using an open file descriptor
// returns 0 if it was successful or 1 if it failed
int write_integer(int file_descriptor, void* integer, int size_of_integer);

// reads c string from a file using an open file descriptor
// returns size of the string or -1 if it failed
// set *size to a size of the string
ssize_t read_string(int file_descriptor, char** buf, unsigned int* size, unsigned int max_size);

// writes c string to a file using an open file descriptor
// returns size of the string or -1 if it failed
ssize_t write_string(int file_descriptor, char* buf, unsigned int size);

// parses string to int
// fails if the value is smaller than min or bigger than max
// returns 0 if the parsing was successful
// return 1 if the parsing failed
int parse_int(const char* text, int* result, int min, int max);

// returns 0 if string is a valid path and 1 otherwise
enum is_valid_path_mode { IS_VALID_ALL, IS_VALID_FILE, IS_VALID_DIR, IS_VALID_LINK, IS_VALID_REGF };
int is_valid_path(const char* path_candidate, enum is_valid_path_mode mode);

#endif
