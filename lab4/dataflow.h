#ifndef dataflow_h
#define dataflow_h

#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>

typedef struct cfg_t cfg_t;

typedef enum {
	IN,
	OUT,
	USE,
	DEF,
	NSETS
} set_type_t;

cfg_t*	new_cfg(size_t nvertex, size_t nsymbol, size_t max_succ);
void	free_cfg(cfg_t*);

void 	connect(cfg_t* cfg, size_t pred, size_t succ);
void	liveness(cfg_t*);

bool	testbit(cfg_t*, size_t vertex, set_type_t type, size_t index);
void	setbit(cfg_t*, size_t vertex, set_type_t type, size_t index);
void	print_sets(cfg_t*, FILE*);

#endif
