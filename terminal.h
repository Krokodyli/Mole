#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <libgen.h>

#define MIN_T_ARG 30
#define MAX_T_ARG 7200
#define MOLE_DEFAULT_FILE ".mole-index"

#include "moleIO.h"
#include "string.h"
#include "moleData.h"
#include "indexer.h"
#include "streamHandler.h"

// prints prompt and passes input to parse_and_run_command function
void run_terminal(mole_data_t *data);

// parses program parameters and saves them to mole_data_t structure
int parse_args(int argc, char** argv, mole_data_t *data);

// removes '/' character from the end of string
void fix_path(char* path);

// parses program parameter from optarg if it is not NULL
// otherwise it tries to set parameter to default value
int parse_path_d(mole_data_t *data, char *optarg);
int parse_path_f(mole_data_t *data, char *optarg);

// tries to read values from enviromental variables
// or sets them to default values
int set_path_d_default(mole_data_t *data);
int set_path_f_default(mole_data_t *data);


// commands handling
// ---------------------------------------------------------------
// such design allows to easily extend this project in future
// without modifying existing code a lot
typedef int (*command_func_t)(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options);

typedef struct command {
	char* command_text;
	command_func_t command_func;
} command_t;
// ---------------------------------------------------------------


// parsing
// ---------------------------------------------------------------
// I think the name is really self-explanatory
// returns 1 if program is requested to exit
// otherwise returns 0
// it handles memory deallocation in case of exit request
int parse_and_run_command(mole_data_t *data, char *cmd_buf, unsigned int cmd_len);
void perform_search(mole_data_t *data, search_options_t search_options);

// sets word to the beginning of the next word
// sets p to the current position in text
// sets character after a word to 0
int next_word(char** p, char** word);

// does similiar thing to next_word function but
// it can parse "text like \"this\" "
int next_word_quotation(char **p, char** word);

// removes escaping sequences like \" or double backslash
void delete_escaping_sequences(char *p);
// ---------------------------------------------------------------


// available commands
// they do not perform action!
// they only modify structures passed to them as arguments
// ---------------------------------------------------------------
extern command_t terminal_commands[];

int index_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options);

int exit_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options);

int exit_now_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options);

int owner_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options);

int largerthan_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options);

int namepart_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options);

int count_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options);

int help_command(char** p, char **word, int* parse_status, mole_data_t* data, search_options_t* search_options);
// ---------------------------------------------------------------

void show_help_message();

#endif
