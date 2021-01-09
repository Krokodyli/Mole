#include "streamHandler.h"

int init_search_output_stream(search_output_stream_t* stream){
	stream->length = 0;
	stream->lines = 0;
	stream->buf[0] = 0;
	stream->stream = fmemopen(stream->buf, LINES_BUFFERED*MAX_PATH_SIZE*2, "w");
	if(stream->stream == NULL) return 1;
	return 0;
}

int update_search_output_stream(search_output_stream_t* stream){
	stream->lines++;
	if(stream->lines == LINES_BUFFERED+1){
		if(fclose(stream->stream)) return 1;
		char* pager_val = getenv("PAGER");
		if(pager_val != NULL){
			stream->stream = popen(pager_val, "w");
			if(stream->stream == NULL) stream->stream = stdout;
		}
		else stream->stream = stdout;
		fprintf(stream->stream, "%s", stream->buf);
	}
	return 0;
}

int destroy_search_output_stream(search_output_stream_t* stream){
	if(stream->stream == NULL) return 0;
	if(stream->lines <= LINES_BUFFERED){
		if(fclose(stream->stream)) return 1;
		fprintf(stdout, "%s", stream->buf);
	}
	else {
		if(fflush(stream->stream)) return 1;
		if(stream->stream == stdout) return 0;
		if(pclose(stream->stream)) return 1;
	}
	return 0;
}
