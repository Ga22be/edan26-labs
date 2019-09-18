#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include "set.h"
#include "error.h"

set_t* new_set(size_t m)
{
	set_t*	s;
	size_t	n;

	n = (m + 63) / 64;
	s = calloc(1, sizeof(set_t) + n * sizeof(uint64_t));

	if (s == NULL)
		error("out of memory");

	s->n = n;

	return s;
}

void free_set(set_t* s)
{
	free(s);
}

void set(set_t* s, uint64_t a)
{
	s->a[a / 64] |= 1ULL << (a % 64);
}

void reset(set_t* s)
{
	memset(s->a, 0, s->n * sizeof s->a[0]);
}

bool equal(set_t* a, set_t* b)
{
	return memcmp(a->a, b->a, a->n * sizeof a->a[0]) == 0;
}
		
void or(set_t* t, set_t* a, set_t* b)
{
	size_t	i;

	for (i = 0; i < t->n; ++i)
		t->a[i] = a->a[i] | b->a[i];
}
		
void propagate(set_t* in, set_t* out, set_t* def, set_t* use)
{
	size_t	i;

	for (i = 0; i < in->n; ++i)
		in->a[i] = (out->a[i] & ~def->a[i]) | use->a[i];
}
		
bool test(set_t* s, uint64_t a)
{
	return s->a[a / 64] & (1ULL << (a % 64));
}

void print_set(set_t* s, FILE *fp)
{
	size_t		i;

	if (fp == NULL)
		fp = stderr;

	if (s == NULL) {
		fprintf(fp, "{ }\n");
		return;
	}
	fprintf(fp, "{ ");
	for (i = 0; i < s->n * 64; ++i)
		if (test(s, i))
			fprintf(fp, "%zu ", i);
	fprintf(fp, "}\n");
}
