
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "language.h"

static inline int num(char c)
{
	return (c >= '0' && c <= '9');
}

static inline int alnum(char c)
{
	return (c >= 'a' && c <= 'z') || 
		(c >= 'A' && c <= 'Z') || 
		num(c);
}

void push_token(struct token t, struct program *p)
{
	p->contents = realloc(p->contents, sizeof(struct token) * (++p->size + 1));
	p->contents[p->size - 1] = t;
}

literal parse_num(FILE *input);
struct token parse_id(FILE *input);
struct token parse_op(char input);

struct program parse(FILE *input)
{
	errno = 0;

	struct program result = {0};
	result.sample_rate = 8000;

	int c;

	int comment = 0;
	while ((c = getc(input)) != EOF)
	{
		if (comment)
		{
			if (c == '\n') comment = 0;
			continue;
		}

		struct token t;
		if (num(c))
		{
			ungetc(c, input); 
			literal n = parse_num(input); 
			t.type = lit;
			t.lit = n;
		}
		else if (alnum(c))
		{
			ungetc(c, input);
			t = parse_id(input);
		}
		else if (c == '#')
		{
			comment = 1;
		}
		else if (c == '\\')
		{
			int s = parse_num(input);
			result.sample_rate = s;
		}
		else
		{
			t = parse_op(c);
		}

		if (t.type != empty)
			push_token(t, &result);
	}
	if (errno != 0)
	{
		perror("error reading file");
		struct program e = {0};
		return e;
	}

	return result;
}

literal parse_num(FILE *input)
{
	char c;
	literal i = 0;
	while ((c = getc(input)) != EOF && num(c))
	{
		i *= 10;
		i += c - '0';
	}
	ungetc(c, input);

	return i;
}

struct token parse_id(FILE *input)
{
	char c;
	identifier i = {0};
	for (int x=0; (c = getc(input)) != EOF && alnum(c) && x < sizeof(i); x++)
		i[x] = c;
	ungetc(c, input);
	
	struct token t;
	t.type = id;
	strcpy(t.id, i);
	return t;
}

struct token parse_op(char c)
{
	enum lang_op o;
	switch (c)
	{
		case '+': o = add; break;
		case '-': o = subtract; break;
		case '*': o = multiply; break;
		case '/': o = divide; break;
		case '%': o = modulo; break;
		case '&': o = bitw_and; break;
		case '|': o = bitw_or; break;
		case '^': o = bitw_xor; break;
		case '~': o = bitw_not; break;
		case '@': o = bitw_nand; break;
		case '<': o = bitw_shiftl; break;
		case '>': o = bitw_shiftr; break;
		case '=': o = equals; break;
		case '_': o = negative; break;
		case ':': o = assign; break;
		case '?': o = select_op; break;
		default: o = no_op; break;
	}
	struct token t = {op, o};
	if (o == no_op) t.type = empty; // There was no operator
	return t;
}
