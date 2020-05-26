
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "language.h"

literal stack[256];
size_t s_top = 0;

int error = 0;

struct variable
{
	identifier name;
	literal value;
};

struct variable heap[2048];
size_t h_top = 0;

void push(literal i)
{
	if (s_top < sizeof(stack)/sizeof(literal))
		stack[s_top++] = i;
	else
	{
		fprintf(stderr, "stack overflow\n");
		error = 1;
	}
}

literal pop()
{
	if (s_top > 0)
		return stack[--s_top];
	else
	{
		fprintf(stderr, "stack underflow\n");
		error = 1;
	}
	return 0;
}

size_t find(identifier name)
{
	size_t result = -1;
	for (int i = 0; i < h_top; i++)
	{
		if (strcmp(name, heap[i].name) == 0)
			result = i;
	}
	return result;
}

literal get(identifier name)
{
	size_t i;
	if ((i = find(name)) != (size_t)-1)
		return heap[i].value;
	else
	{
		fprintf(stderr, "undefined variable: %s\n", name);
		error = 1;
	}
	return 0;
}

void set(identifier name, literal value)
{
	size_t i;
	if ((i = find(name)) == (size_t)-1) 
	{
		// The variable is not in the heap, so create it
		if (h_top < sizeof(heap)/sizeof(struct variable))
			i = h_top++;
		else
		{
			fprintf(stderr, "heap overflow on allocating %s\n", name);
			error = 1;
			return;
		}
	}
	strcpy(heap[i].name, name);
	heap[i].value = value;
}

void eval_op(enum lang_op o)
{
	static int zwarning = 0;

	int a,b;
	#define OPERATION(OP) b = pop(); a = pop(); push(a OP b); break;
	switch (o)
	{
		case add:
			OPERATION(+);
		case subtract:
			OPERATION(-);
		case multiply:
			OPERATION(*);
		case divide:
			b = pop(); a = pop();
			if (b == 0) 
			{
				if (!zwarning) fprintf(stderr, "warning: division by zero\n");
				zwarning = 1;
				push(0);
			}
			else
				push(a / b);
			break;
		case modulo:
			OPERATION(%);
		case bitw_and:
			OPERATION(&);
		case bitw_or:
			OPERATION(|);
		case bitw_xor:
			OPERATION(^);
		case bitw_not:
			push(~pop());
			break;
		case bitw_nand:
			push(~(pop() & pop()));
			break;
		case bitw_shiftl:
			OPERATION(<<);
		case bitw_shiftr:
			OPERATION(>>);
		case equals:
			OPERATION(==);
		case negative:
			push(-pop());
			break;
		case assign:
			fprintf(stderr, "invalid use of assignment operator\n");
			error = 1;
			break;
		case select_op:
			b = pop(); a = pop();
			a %= b;
			if (s_top - a <= 0)
			{
				fprintf(stderr, "stack underflow\n");
				s_top = 0;
				error = 1;
				break;
			}
			s_top -= b;
			push(stack[s_top + a]);
			break;
		default:
			//fprintf(stderr, "invalid operator (this is a bug. it shouldn't happen)\n");
			break;
	}
}

literal eval(struct program p, literal t)
{
	h_top = 0;
	s_top = 0;
	error = 0;

	set("t", t);

	int i;
	for (i = 0; i < p.size; i++)
	{
		switch (p.contents[i].type)
		{
			case op:
				if (p.contents[i].op == assign)
				{
					i++;
					if (i > p.size || p.contents[i].type != id)
					{
						fprintf(stderr, "expected identifier after assignment\n");
						error = 1;
					}
					else
					{
						set(p.contents[i].id, pop());
					}
				}
				else
					eval_op(p.contents[i].op);
				break;
			case id:
				push(get(p.contents[i].id));
				if (error) fprintf(stderr, "on pushing '%s'\n", p.contents[i].id);
				break;
			case lit:
				push(p.contents[i].lit);
				if (error) fprintf(stderr, "on pushing %d\n", p.contents[i].lit);
				break;
			case empty:
				fprintf(stderr, "bad error\n");
				break;
		}
		if (error) break;
	}

	return pop();
}
