// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "alloc.h"
#include <ctype.h>
#include <error.h>
#include <stdio.h>
#include "command-internals.h"

typedef struct Node {
	struct command payload;
	struct Node* next;
} Node;

typedef struct command_stream {
	command_stream();
    ~command_stream();
    struct command **arr;
	struct command **heads;
	int cur;
} command_stream;

command_stream::command_stream()
{
    **arr = checked_malloc(512);
    **heads = checked_malloc(512);
}

command_stream::~command_stream()
{
    free(arr);
    free(heads);
}

void parse(int (*get_next_byte)(void *), 
		void *get_next_byte_argument) 
{}

void tree()
{}

struct command_stream stream;

bool isSimple(char c) {
	if(isalpha(c))
		return true;
	if(isdigit(c))
		return true;
	if(c=='!' || c=='%' || c=='+' || c==',' ||
		c=='-' || c=='.' || c=='/' || c==':' ||
		c=='@' || c=='^' || c=='_')
		return true;
	return false;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
	parse(get_next_byte, get_next_byte_argument);
	if(stream.array == 0)
		return 0;
	tree();
	return &stream;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}
