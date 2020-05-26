#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <stddef.h>

enum lang_op
{
	add,
	subtract,
	multiply,
	divide,
	modulo,
	bitw_and,
	bitw_or, 
	bitw_xor,
	bitw_not,
	bitw_nand,
	bitw_shiftl,
	bitw_shiftr,
	equals,
	negative,
	assign,
	select_op,
	no_op
};

typedef char identifier[32];
typedef int literal;

struct token
{
	enum {op, id, lit, empty} type;
	union {enum lang_op op; identifier id; literal lit;};
};

struct program
{
	size_t size;
	int sample_rate;
	struct token *contents;
};

int error;
int eval(struct program p, literal t);

#endif
