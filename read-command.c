// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "alloc.h"
#include <stdlib.h>
#include <ctype.h>
#include <error.h>
#include <stdio.h>
#include "command-internals.h"

typedef struct command_stream {
	struct command **array;
	struct command **heads;
	int cur;
    int numTree;
    int numEntry;
} command_stream;

struct command_stream stream;
typedef struct Node
{
    struct command* cmd;
    Node* next;
} Node;

void
s_push(Stack stack, struct command* cmd)
{
    Node* new = checked_malloc(sizeof(Node));
    new->cmd = cmd;
    new->next = stack.head;
    stack.head = new;
}

struct command*
s_peak(Stack stack)
{
    struct command* cmd = stack.head->cmd;
    return cmd;
}

struct command*
s_pop(Stack stack)
{
    if (stack.head == NULL)
        return NULL; /* stack is empty */

    struct command* cmd = stack.head->cmd;
    Node* tmp = stack.head;
    stack.head = stack.head->next;
    free(tmp);
    return cmd;
}

typedef struct
Stack
{
    Node* head;
} Stack;

Stack cstack;
Stack opstack;

void
tree()
{
    int i;
    for (i = 0; i < stream.numEntry; i++)
    {
        command* seq;
        while(stream.array[i] != NULL)
        {
            command* cmd = stream.array[i];
            /* Linking different commands */
            switch (cmd->type) {
                case SIMPLE_COMMAND:
                    /* push command on cstack */
                    s_push(cstack, cmd)
                    break;
                case AND_COMMAND:
                    opHandler(cmd);
                    break;
                case OR_COMMAND:
                    opHandler(cmd);
                    break;
                case PIPE_COMMAND:
                    opHandler(cmd);
                    break;
                case SEQUENCE_COMMAND:
                    /* clear stacks */
                    command* subhead = clearStack();
                    /* link sequence command */
                    if (seq)
                    {
                        seq->u.command[1] = cmd->u.command[0] = subhead;
                        seq = subhead;
                    }
                    break;
                case SUBSHELL_COMMAND:
                    /* if '(' push */
                    if (cmd->input == 1 && cmd->output ==0)
                    {
                        s_push(opstack, cmd);
                        break;
                    }
                    /* if ')'  */
                    if (cmd->input == 0 && cmd->output ==1)
                    {
                        opHandler(cmd);
                        break;
                    }
                    break;
            }
        }
    }
    
    clearStack();
    
    if ( treeCount != stream.numEntry)
        printf("treeCount error %d, should be %d",treeCount, stream.numEntry);

}

void
opHandler(command* cmd)
{
    if (opstack.head == NULL)
    {
        s_push(opstack, cmd);
        return;
    }
    
    while (operatorPrecedence(s_peak(opstack), cmd->type) && opstack.head != NULL)
    {
        if (cmd->type == SUBSHELL_COMMAND &&
            s_peak(opstack)->type == SUBSHELL_COMMAND &&
            s_peak(opstack)->input == 1 && s_peak(opstack)->output == 0)
        {
            command* subshell = s_pop(opstack);
            subshell->u.subshell_command = s_pop(cstack);
            s_push(cstack, subshell);
            return;
        }

        command* top = s_pop(opstack);
        command* cmd2 = s_pop(cstack);
        command* cmd1 = s_pop(cstack);
        top->u.command[0] = cmd1;
        top->u.command[1] = cmd2;
        s_push(cstack, top);
    }
    
    s_push(opstack, cmd);
}

command*
clearStack(void)
{
    while (opstack.head != NULL && cstack.head != NULL) {
        command* top = s_pop(opstack);
        command* cmd2 = s_pop(cstack);
        command* cmd1 = s_pop(cstack);
        top->u.command[0] = cmd1;
        top->u.command[1] = cmd2;
        s_push(cstack, top);
    }
}

bool operatorPrecedence (enum command_type c1, enum command_type c2)
{
    /* set value to c1 based on precedence */
    switch (c1) {
        case SUBSHELL_COMMAND:
            c1 = 0;
        case AND_COMMAND:
        case OR_COMMAND:
            c1 = 1; break;
        case PIPE_COMMAND:
            c1 = 2; break;
    }
    /* set value to c1 based on precedence */
    switch (c2) {
        case SUBSHELL_COMMAND:
            c2 = 0;
        case AND_COMMAND:
        case OR_COMMAND:
            c2 = 1; break;
        case PIPE_COMMAND:
            c2 = 2; break;
    }
    
    /* return true if c1 has equal or higher precedence 
     * with regard to c2 
     */
    return c1 >= c2;
}

void initialize() {
	stream.array = checked_malloc(sizeof(struct command*));
	stream.cur = 0;
    stream.numTree = 0;
    stream.numEntry = 0;
    cstack.head = opstack.head = NULL;
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
                                   (stream.cur+1) * sizeof(struct command*));
	stream.array[stream.cur] = NULL;
	stream.cur++;
    stream.numTree++;
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
    stream.numEntry = stream.cur;
    stream.cur = 0;
}

command_stream_t make_command_stream(int(*get_next_byte) (void *),
                                     void *get_next_byte_argument)
{
	parse(get_next_byte, get_next_byte_argument);
	if (stream.numEntry == 0)
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
