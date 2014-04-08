nclude "command.h"
#include "alloc.h"
#include <stdlib.h>
#include <ctype.h>
#include <error.h>
#include <stdio.h>
#include "command-internals.h"

typedef struct Node {
	struct command payload;
	struct Node* next;
} Node;

typedef struct command_stream {
	struct command **array;
	struct command **heads;
	int cur;
} command_stream;

struct command_stream stream;

void tree()
{}

void initialize() {
	stream.array = checked_malloc(sizeof(struct command*));
	stream.cur = 0;
}

struct command newCommand(enum command_type type){
	struct command temp;
	temp.type = type;
	temp.status = -1;
	temp.input = NULL;
	temp.output = NULL;
	return temp;
}

void addCommand(struct command cmd) {
	stream.array = checked_realloc(stream.array,
		(stream.cur + 1) * sizeof(struct command*));
	stream.array[stream.cur] = checked_malloc(sizeof(struct command));
	*stream.array[stream.cur] = cmd;
	stream.cur++;
	return;
}

void newTree() {
	stream.array = checked_realloc(stream.array,
		(stream.cur + 1) * sizeof(struct command*));
	stream.array[stream.cur] = 0;
	stream.cur++;
	return;
}


bool isSimple(char c) {
	if (isalpha(c))
		return true;
	if (isdigit(c))
		return true;
	if (c == '!' || c == '%' || c == '+' || c == ',' ||
		c == '-' || c == '.' || c == '/' || c == ':' ||
		c == '@' || c == '^' || c == '_')
		return true;
	return false;
}


void parse(int(*get_next_byte)(void *),
	void *get_next_byte_argument)
{
	initialize();
	enum command_type lastType;
	char c = get_next_byte(get_next_byte_argument);
	while (c == ' ' || c == 9 || c == '\n')
		c = get_next_byte(get_next_byte_argument);
	if (c == EOF)
		return;
	while (c != EOF) {
		if (c == '#') {
			while (c != '\n')
				c = get_next_byte(get_next_byte_argument);
		}
		if (isSimple(c)) {
			int num_words = 0;
			int num_letters = 0;
			struct command temp = newCommand(SIMPLE_COMMAND);
			temp.u.word = checked_malloc(sizeof(char*));
			char *cur_string = checked_malloc(sizeof(char));
			temp.u.word[0] = cur_string;
			while (isSimple(c) || c == 9 || c == ' ' ||
				c == '>' || c == '<') {
				if (isSimple(c)) {
					cur_string = checked_realloc(cur_string,
						((num_letters + 1)* sizeof(char)));
					cur_string[num_letters] = c;
					num_letters++;
				}
				else if (c == 9 || c == ' ') {
					cur_string = checked_realloc(cur_string,
						((num_letters + 1) * sizeof(char)));
					cur_string[num_letters] = '\0';
					num_letters = 0;
					num_words++;
					temp.u.word = checked_realloc(temp.u.word,
						((num_words + 1) * sizeof(char*)));
					cur_string = checked_malloc(sizeof(char));
					temp.u.word[num_words] = cur_string;
				}
				c = get_next_byte(get_next_byte_argument);
			}
			if (num_letters != 0) {
				cur_string = checked_realloc(
					cur_string, (num_letters + 1)*sizeof(char));
				cur_string[num_letters] = '\0';
				num_words++;
				temp.u.word = checked_realloc(temp.u.word,
					(num_words + 1)*sizeof(char*));
				temp.u.word[num_words] = 0;
			}
			else {
				temp.u.word[num_words] = 0;
				free(cur_string);
			}
			while (c == ' ' || c == 9)
				c = get_next_byte(get_next_byte_argument);

			if (c == '<') {
				char* input_string = checked_malloc
					(sizeof(char));
				int letters = 0;
				while (c == '<' || c == ' ' || c == 9) {
					c = get_next_byte(
						get_next_byte_argument);
				}
				//error code goes here
				while (isSimple(c)) {
					input_string = checked_realloc(
						input_string,
						(letters + 1)*sizeof(char));
					input_string[letters] = c;
					letters++;
					c = get_next_byte(
						get_next_byte_argument);
				}
				input_string[letters] = '\0';
				temp.input = input_string;
			}
			while (c == ' ' || c == 9)
				c = get_next_byte(get_next_byte_argument);

			if (c == '>') {
				char* output_string = checked_malloc
					(sizeof(char));
				int letters = 0;
				while (c == '>' || c == ' ' || c == 9) {
					c = get_next_byte(
						get_next_byte_argument);
				}
				while (isSimple(c)) {
					output_string = checked_realloc(
						output_string,
						(letters + 1)*sizeof(char));
					output_string[letters] = c;
					letters++;
					c = get_next_byte(
						get_next_byte_argument);
				}
				output_string[letters] = '\0';
				temp.output = output_string;
			}
			addCommand(temp);
			lastType = SIMPLE_COMMAND;
			continue;

		}//end of isSimple
		if (c == '|') {
			struct command temp;
			c = get_next_byte(get_next_byte_argument);
			if (c == '|') {
				temp = newCommand(OR_COMMAND);
				addCommand(temp);
				lastType = OR_COMMAND;
			}
			else {
				temp = newCommand(PIPE_COMMAND);
				addCommand(temp);
				lastType = PIPE_COMMAND;
				continue;
			}
		}
		if (c == '&'){
			struct command temp;
			c = get_next_byte(get_next_byte_argument);
			if (c == '&') {
				temp = newCommand(AND_COMMAND);
				addCommand(temp);
				lastType = AND_COMMAND;
			}
			else {
				//error(one &);
			}
		}
		if (c == '\n') {
			if (lastType == PIPE_COMMAND || lastType == AND_COMMAND ||
				lastType == OR_COMMAND ||
				lastType == SEQUENCE_COMMAND) {
			}
			else {
				c = get_next_byte(get_next_byte_argument);
				while (c == ' ' || c == 9)
					c = get_next_byte(get_next_byte_argument);
				if (c == '\n') {
					newTree();
				}
				else {
					struct command temp;
					temp = newCommand(SEQUENCE_COMMAND);
					addCommand(temp);
					continue;
				}
			}
		}
		if (c == '(') {
			struct command temp = newCommand(SUBSHELL_COMMAND);
			temp.input = (char*)1;
			addCommand(temp);
		}
		if (c == ')') {
			struct command temp = newCommand(SUBSHELL_COMMAND);
			temp.output = (char*)1;
			addCommand(temp);
		}
		c = get_next_byte(get_next_byte_argument);
		while (c == ' ' || c == 9) {
			c = get_next_byte(get_next_byte_argument);
		}
	}
}

command_stream_t make_command_stream(int(*get_next_byte) (void *),
	void *get_next_byte_argument)
{
	parse(get_next_byte, get_next_byte_argument);
	if (stream.array == 0)
		return 0;
	tree();
	return &stream;
}

command_t
read_command_stream(command_stream_t s)
{
	/* FIXME: Replace this with your implementation too.  */
	error(1, 0, "command reading not yet implemented");
	return 0;
}

