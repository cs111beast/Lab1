// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "alloc.h"
#include <ctype.h>
#include <error.h>
#include <stdio.h>
#include "command-internals.h"

typedef struct Node
{
	struct command cmd;
	struct Node* left;
    struct Node* right;
} Node;

typedef struct stack
{
    Node* obj;
    stack* next;
} stack;

stack*
s_push(stack** head, stack* new, Node* node)
{
    new->obj = node;
    new->next = head;
    *head = new;
}

stack*
s_pop(stack* head)
{
    if (head != NULL)
    {
        head = head->next;
    }
    return head;
}

typedef struct command_stream
{
    struct Node** arr;
    int size;
	int pos;
} command_stream;


void
parse(int (*get_next_byte)(void *),
		void *get_next_byte_argument) 
{}

void
tree()
{
    int i;
    for (i = 0; i < command_stream.size; i++)
    {
        Node* cur = command_stream.arr[i];
        Node* next = cur->left;
        
        stack head = NULL;
        
        /* if more than one simple command */
        while (cur != NULL && next != NULL)
        {
            next = cur->left;
            switch (cur->cmd.type) {
                case SIMPLE_COMMAND:
                    /* push on command stack */
                    
                    continue; break;
                case AND_COMMAND:
                    
                    break;
                case OR_COMMAND:
                    break;
                case PIPE_COMMAND:
                    break;
                case SEQUENCE_COMMAND:
                    /* clear stacks */
                    break;
                case SUBSHELL_COMMAND:
                    /* do something */
                    break;
            }
            
            
            cur = next;
        }
    }

}

bool operatorPrecedence (enum command_type c1, enum command_type c2)
{
    /* set value to c1 based on precedence */
    switch (c1) {
        case AND_COMMAND:
        case OR_COMMAND:
            c1 = 1; break;
        case PIPE_COMMAND:
            c1 = 2; break;
    }
    /* set value to c1 based on precedence */
    switch (c2) {
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
