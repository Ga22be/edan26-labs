#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include "dataflow.h"
#include "error.h"
#include "list.h"
#include "set.h"

typedef struct vertex_t	vertex_t;
typedef struct task_t	task_t;

/* cfg_t: a control flow graph. */
struct cfg_t {
	size_t			nvertex;	/* number of vertices		*/
	size_t			nsymbol;	/* width of bitvectors		*/
	vertex_t*		vertex;		/* array of vertex		*/
};

/* vertex_t: a control flow graph vertex. */
struct vertex_t {
	size_t			index;		/* can be used for debugging	*/
	set_t*			set[NSETS];	/* live in from this vertex	*/
	set_t*			prev;		/* alternating with set[IN]	*/
	size_t			nsucc;		/* number of successor vertices */
	vertex_t**		succ;		/* successor vertices 		*/
	list_t*			pred;		/* predecessor vertices		*/
	bool			listed;		/* on worklist			*/
	pthread_mutex_t		mutex;
};

pthread_mutex_t worklistMutex = PTHREAD_MUTEX_INITIALIZER;

static void clean_vertex(vertex_t* v);
static void init_vertex(vertex_t* v, size_t index, size_t nsymbol, size_t max_succ);

cfg_t* new_cfg(size_t nvertex, size_t nsymbol, size_t max_succ)
{
	size_t		i;
	cfg_t*		cfg;

	cfg = calloc(1, sizeof(cfg_t));
	if (cfg == NULL)
		error("out of memory");

	cfg->nvertex = nvertex;
	cfg->nsymbol = nsymbol;

	cfg->vertex = calloc(nvertex, sizeof(vertex_t));
	if (cfg->vertex == NULL)
		error("out of memory");

	for (i = 0; i < nvertex; i += 1)
		init_vertex(&cfg->vertex[i], i, nsymbol, max_succ);

	return cfg;
}

static void clean_vertex(vertex_t* v)
{
//	pthread_mutex_lock(&v->mutex); // TODO check for error
	int		i;

	for (i = 0; i < NSETS; i += 1)
		free_set(v->set[i]);
	free_set(v->prev);
	free(v->succ);
	free_list(&v->pred);
//	pthread_mutex_unlock(&v->mutex); // TODO check for error
//	pthread_mutex_destroy(&v->mutex); // TODO check for error
}

static void init_vertex(vertex_t* v, size_t index, size_t nsymbol, size_t max_succ)
{
//	pthread_mutexattr_t mutexattr;
//	pthread_mutexattr_init(&mutexattr);
//	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);

//	int ret = pthread_mutex_init(&v->mutex, &mutexattr);
//	if (ret != 0) {
//		fprintf(stderr, "Failed to init mutex of vertex %zu: %i\n", index, ret);
//		exit(EXIT_FAILURE);
//	}

//	pthread_mutex_lock(&v->mutex); // TODO check for error
	int		i;

	v->index	= index;
	v->succ		= calloc(max_succ, sizeof(vertex_t*));

	if (v->succ == NULL)
		error("out of memory");
	
	for (i = 0; i < NSETS; i += 1)
		v->set[i] = new_set(nsymbol);

	v->prev = new_set(nsymbol);
//	pthread_mutex_unlock(&v->mutex); // TODO check for error
}

void free_cfg(cfg_t* cfg)
{
	size_t		i;

	for (i = 0; i < cfg->nvertex; i += 1)
		clean_vertex(&cfg->vertex[i]);
	free(cfg->vertex);
	free(cfg);
}

void connect(cfg_t* cfg, size_t pred, size_t succ)
{
	vertex_t*	u;
	vertex_t*	v;

	u = &cfg->vertex[pred];
	v = &cfg->vertex[succ];

	u->succ[u->nsucc++ ] = v;
	insert_last(&v->pred, u);
}

bool testbit(cfg_t* cfg, size_t v, set_type_t type, size_t index)
{
	return test(cfg->vertex[v].set[type], index);
}

void setbit(cfg_t* cfg, size_t v, set_type_t type, size_t index)
{
	set(cfg->vertex[v].set[type], index);
}

vertex_t* pop_worklist(list_t** worklist)
{
	vertex_t* u;
	pthread_mutex_lock(&worklistMutex); // TODO check for error
	u = (vertex_t*)remove_first(worklist);
	if (u != NULL) {
//		pthread_mutex_lock(&u->mutex); // TODO check for error
//		u->listed = false;
//		pthread_mutex_unlock(&u->mutex); // TODO check for error
	}
	pthread_mutex_unlock(&worklistMutex); // TODO check for error
	return u;
}	

void put_in_worklist(list_t** worklist, vertex_t* u)
{
	pthread_mutex_lock(&worklistMutex); // TODO check for error
//	pthread_mutex_lock(&u->mutex); // TODO check for error
//	u->listed = true;
//	pthread_mutex_unlock(&u->mutex); // TODO check for error
	insert_last(worklist, u);
	pthread_mutex_unlock(&worklistMutex); // TODO check for error
}

void* work(void* arg)
{
	vertex_t*	u; // "ref" for vertex from worklist
	vertex_t*	v; // "ref" for predecessors of u
	set_t*		prev; // switch variable for u->prev
	size_t		j; // index variable: 0 to u->nsucc
	list_t**	worklist = (list_t**)arg;
	list_t*		p; // iterator for u->pred
	list_t*		h; // end condition for p

	if (arg==NULL) {
		fprintf(stderr, "Worklist is null!\n");
		exit(EXIT_FAILURE);
	}

	// while worklist not empty
	while ((u = pop_worklist(worklist)) != NULL) {
		pthread_mutex_lock(&u->mutex); // TODO check for error
		
		u->listed = false;
		reset(u->set[OUT]);

		pthread_mutex_unlock(&u->mutex);

		// for each successor j
		// 	u.out |= j.in

		// ============================= MUCHO IMPORTANTE ===========================
		// added mutex but forgot {} !!!!!!
		for (j = 0; j < u->nsucc; ++j) {
			pthread_mutex_lock(&u->succ[j]->mutex);			
			or(u->set[OUT], u->set[OUT], u->succ[j]->set[IN]);
			pthread_mutex_unlock(&u->succ[j]->mutex);	
		}


		pthread_mutex_lock(&u->mutex);	

		prev = u->prev;
		u->prev = u->set[IN];
		u->set[IN] = prev;

		/* in our case liveness information... */
		// in = use U (out - def)
		propagate(u->set[IN], u->set[OUT], u->set[DEF], u->set[USE]);

		pthread_mutex_unlock(&u->mutex); // TODO check for error

		// if change
		// 	add all predecessors to worklist if they aren't
		if (u->pred != NULL && !equal(u->prev, u->set[IN])) {
			p = h = u->pred;
			do {
				v = p->data;
				pthread_mutex_lock(&v->mutex);				
				if (!v->listed) {
					v->listed = true;
					put_in_worklist(worklist, v);
				}
				pthread_mutex_unlock(&v->mutex);			
				p = p->succ;

			} while (p != h);
		}
	}

	return NULL;
}

void liveness(cfg_t* cfg)
{
	vertex_t*	u; // "ref" for vertex from worklist
	size_t		i; // index variable: 0 to cfg->nvertex
	list_t*		worklist;

	worklist = NULL;

	// put each vertex in cfg into worklist
	for (i = 0; i < cfg->nvertex; ++i) {
		u = &cfg->vertex[i];

		insert_last(&worklist, u);
		u->listed = true;
	}

	size_t		nThreads = 2;
	printf("nthreads  = %zu\n", nThreads);
	pthread_t	thread[nThreads];

	// Start all workers
	int create_err = 0;
	for (size_t index = 0; index < nThreads; index++) {
		create_err = pthread_create(&thread[index], NULL, work, &worklist);
		if (create_err != 0) {
			fprintf(stderr, "Failed to create thread[%zu]: %i\n", index, create_err);
			exit(EXIT_FAILURE);
		}
	}

	// Join all workers
	int join_err = 0;
	for (size_t index = 0; index < nThreads; index++) {
		join_err = pthread_join(thread[index], NULL);
		if (join_err != 0) {
			fprintf(stderr, "Failed to join thread[%zu]: %i\n", index, join_err);
			exit(EXIT_FAILURE);
		}
	}
}

void print_sets(cfg_t* cfg, FILE *fp)
{
	size_t		i;
	vertex_t*	u;

	for (i = 0; i < cfg->nvertex; ++i) {
		u = &cfg->vertex[i]; 
		fprintf(fp, "use[%zu] = ", u->index);
		print_set(u->set[USE], fp);
		fprintf(fp, "def[%zu] = ", u->index);
		print_set(u->set[DEF], fp);
		fputc('\n', fp);
		fprintf(fp, "in[%zu] = ", u->index);
		print_set(u->set[IN], fp);
		fprintf(fp, "out[%zu] = ", u->index);
		print_set(u->set[OUT], fp);
		fputc('\n', fp);
	}
}
