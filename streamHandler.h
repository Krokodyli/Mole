#ifndef STREAMHANDLER_H
#define STREAMHANDLER_H

#include <stdio.h>
#include <stdlib.h>

#define LINES_BUFFERED 3

#include "indexTree.h"

// output stream handler that buffers first lines
// allowing to choose where we want to write our data

// in this case we will write out our output to stdout if there
// are less than LINES_BUFFERED+1 lines
// otherwise we will use $PAGER program to write out our output

typedef struct search_output_stream {
	FILE* stream;
	unsigned int lines;
	unsigned int length;
	char buf[LINES_BUFFERED*MAX_PATH_SIZE*2];
} search_output_stream_t;


int init_search_output_stream(search_output_stream_t* stream);

int update_search_output_stream(search_output_stream_t* stream);

int destroy_search_output_stream(search_output_stream_t* stream);

#endif
