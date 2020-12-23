#ifndef INDEXINGTHREAD_H
#define INDEXINGTHREAD_H

#include <pthread.h>
#include <signal.h>
#include "moleData.h"
#include "indexer.h"

// sets signal handler using sigaction function
int set_signal_handler(void (*f)(int), int sig);

// empty function that "handles" SIGUSR1 signal
void user_signal_handler(int sig);

// thread routine that handles indexing in the background
void* indexing_thread(void* args);

// performs initialization routine before main loop
// of the indexing thread
// returns 1 if it failed, 0 otherwise
int setup_indexing_thread(mole_data_t* data);

// safely replaces current_index_tree with new_index_tree
// keeps mutex locked for as short a period as possible
void replace_current_index_tree(mole_data_t *data);

// safely performs indexing of a directory
// handles mutexes and changes last_indexing_time
int perform_indexing(mole_data_t *data);

// loads data to mole_data_t structure from
// path_f file
// returns 1 if it failed, 0 otherwise
int load_data(mole_data_t *data);

// read modification time of the index file
void load_modification_time(mole_data_t *data);

// tries to load and parse file
// returns 1 if it failed, 0 otherwise
int load_index_file(mole_data_t * data);

// saves data from mole_data_t structure to
// path_f file
// returns 1 if it failed, 0 otherwise
int save_data(mole_data_t *data);


#endif
