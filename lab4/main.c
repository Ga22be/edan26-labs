#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/time.h>
#include "dataflow.h"
#include "list.h"
#include "error.h"
#include "random.h"

char*	progname;

static double sec(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);

	return tv.tv_sec + 1e-6 * tv.tv_usec;
}

static void generate_cfg(cfg_t* cfg, size_t n, int max_succ)
{
	int		i;
	int		j;
	int		k;
	int		s;
	FILE*		fp;
	const char*	name = "cfg.dot";

	fp = fopen(name, "w");

	if (fp == NULL)
		error("cannot open \"%s\" for writing", name);

	fprintf(fp, "digraph cfg {\n");
	fprintf(fp, "0 -> { 1 2 }\n");

	connect(cfg, 0, 1);
	connect(cfg, 0, 2);

	for (i = 2; i < n; ++i) {
		
		s = next() % max_succ;
		s += 1;
		
		for (j = 0; j < s; ++j) {
			k = abs(next()) % n;
			
			connect(cfg, i, k);
			fprintf(fp, "%d -> { %d }\n", i, k);
		}
	}

	fprintf(fp, "}\n");
	fclose(fp);
}

static void generate_usedefs(
	cfg_t*		cfg,
	size_t		n,
	size_t		nsym,
	size_t		nactive)
{
	size_t		i;
	size_t		j;
	size_t		sym;

	for (i = 0; i < n; ++i) {
		for (j = 0; j < nactive; ++j) {
			sym = abs(next()) % nsym;

			if (j % 4 != 0) {
				if (!testbit(cfg, i, DEF, sym))
					setbit(cfg, i, USE, sym);
			} else if (!testbit(cfg, i, USE, sym))
				setbit(cfg, i, DEF, sym);
		}
	}
}

int main(int argc, char** argv) 
{
	double		begin;
	double		end;
	size_t		nsym;
	size_t		nactive;
	size_t		n;
	size_t		max_succ;
	cfg_t*		cfg;
	bool		print;
	int		seed = 0;

	progname	= argv[0];

	if (argc == 7) {
		nsym	 	= atoi(argv[1]);
		n		= atoi(argv[2]);
		max_succ	= atoi(argv[3]);
		nactive	 	= atoi(argv[4]);
		print	 	= atoi(argv[6]);
	} else {
		nsym	 	= 100;
		n		= 10;
		max_succ	= 4;
		nactive	 	= 10;
		print		= 1;
	}

	printf("nsymbol   = %zu\n", nsym);
	printf("nvertex   = %zu\n", n);
	printf("max-succ  = %zu\n", max_succ);
	printf("nactive   = %zu\n", nactive);
	
	if (seed == 1)
		init_random(seed);
	else {
		printf("pid %d\n", getpid());
		init_random(getpid());
	}

	printf("generating cfg...\n");
	cfg = new_cfg(n, nsym, max_succ);
	generate_cfg(cfg, n, max_succ);

	printf("generating usedefs...\n");
	generate_usedefs(cfg, n, nsym, nactive);

	printf("liveness...\n\n");
	begin = sec();
	liveness(cfg);
	end = sec();

	printf("T = %8.4lf s\n\n", end-begin);

	if (print)
		print_sets(cfg, stdout);

	free_cfg(cfg);
	return 0;
}
