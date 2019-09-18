#ifndef list_h
#define list_h

typedef struct list_t	list_t;

struct list_t {
	list_t*		pred;
	list_t*		succ;
	void*		data;
};

list_t*	new_list(void*);
void	free_list(list_t**);
void	apply(list_t*, void (*)(void*));
void	delete_list(list_t*);
void	append(list_t**, list_t*);
void*	remove_first(list_t**);
void*	remove_last(list_t**);
void	insert_before(list_t**, void*);
void	insert_after(list_t**, void*);
void 	insert_last(list_t**, void*);
size_t	length(list_t*);

#endif
