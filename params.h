#ifndef PARAM_H
#define PARAM_H

typedef struct {
	char *key;
	char *val;
} param_t;

typedef struct {
	int capacity;
	int size;
	char *name;
	param_t *p;
} param_group_t;

typedef struct {
	int capacity;
	int size;
	param_group_t *g;
} *params_t;

int make_param_group( char *name, int size );

int add_parameter( char *togroup, char *key, char *val );

int read_param_file( char *filepath, char *group_name );

int write_param_file( char *filepath, char *group_name );

char *get_param( char *group, char *key );

void free_params( void );

#endif
	
