#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include "terminal.h"
#include "indexer.h"
#include "indexingThread.h"


int main(int argc, char** argv){
	mole_data_t data;
	if(init_mole_data(&data)){
		fprintf(stderr, "Could not init data structure. Exiting...\n");
		return EXIT_FAILURE;
	}

	if(parse_args(argc, argv, &data)){
		fprintf(stderr, "Run program with a parameter \"help\" or \"-h\" to show help and short description.\n");
		return EXIT_FAILURE;
	}

	// blocks SIGPIPE signal in case of quiting from $PAGER
	// program before the end of a search
	set_signal_handler(SIG_IGN, SIGPIPE);

	if(pthread_create(&data.indexing_thread_tid, NULL, indexing_thread, &data)){
		printf("pthread_create failed\n");
		return 1;
	}

	run_terminal(&data);

	return EXIT_SUCCESS;
}
