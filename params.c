/* TODO: 
	cleaning:
	 remove_param
	 remove_group
	 remove_all
*/

#include "params.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static params_t all_parms = NULL;

#define START_GROUPS_NO 4
#define START_GROUP_SIZE 16

char *make_string_copy( char *s ) {
	char *cp = (char*)malloc( strlen(s)+1 );
	if( cp != NULL )
		strcpy( cp, s );
	return cp;
}

static int init_all() {
	all_parms = (params_t)malloc( sizeof *all_parms );
	if( all_parms == NULL )
		return 0;
	all_parms->g = (param_group_t*)calloc( START_GROUPS_NO, sizeof *all_parms->g );
	if( all_parms->g == NULL ) {
		free( all_parms );
		return 0;
	}
	all_parms->capacity = START_GROUPS_NO;
	all_parms->size = 0;
	return 1;
}

static int resize_group( param_group_t *g ) {
	param_t * n = (param_t*)realloc( g->p, g->capacity * 2 * sizeof *n );
	if( n == NULL )
		return 0;
	g->p = n;
	g->capacity *= 2;
	return 1;
}

static int resize_all() {
	param_group_t *n = (param_group_t*)realloc( all_parms->g, all_parms->capacity * 2 * sizeof *n );
	if( n == NULL )
		return 0;
	all_parms->g = n;
	all_parms->capacity *= 2;
	return 1;
}

static int init_group( char *name, int size, param_group_t *g ) {
	g->name = make_string_copy( name );
	if( g->name == NULL ) {
		return 0;
	}
	g->p = (param_t*)calloc( size, sizeof * g->p );
	g->size = 0;
	g->capacity = size;
	return 1;
}

static int add2group( param_group_t *g, char *key, char *val ) {
	if( g->size == g->capacity && resize_group( g ) == 0 )
		return 0;
	if( (g->p[g->size].key= make_string_copy( key )) == NULL )
		return 0;
	if( (g->p[g->size].val= make_string_copy( val )) == NULL ) {
		free( g->p[g->size].key );
		return 0;
	}
	g->size++;
	return 1;
}

int param_group_exists( char *name ) {
	int i;
	if( all_parms == NULL ) {
		init_all();
		return 0;
	}
        for( i= 0; i < all_parms->size; i++ )
                if( strcmp( name, all_parms->g[i].name ) == 0 ) {
                        return 1;
                }
        return 0;
}

int make_param_group( char *name, int size ) {
	if( all_parms == NULL && init_all() == 0 )
		return 0;
	if( all_parms->size == all_parms->capacity && resize_all() == 0 )
		return 0;
	if( init_group( name, size, all_parms->g+all_parms->size ) == 0 )
		return 0;

	all_parms->size++;
	return 1;
}

int add_parameter( char *togroup, char *key, char *val ) {
	int i;
	if( all_parms == NULL && init_all() == 0 && make_param_group( togroup, START_GROUP_SIZE ) == 0 )
		return 0;
	for( i= 0; i < all_parms->size; i++ )
		if( strcmp( togroup, all_parms->g[i].name ) == 0 ) {
			return add2group( all_parms->g+i, key, val );
		}
	return 0;
}

int read_param_file( char *filepath, char *group_name ) {
	FILE *in = fopen( filepath, "r" );
	char buf[1024];
	char *key, *val;
	int i;
	if( in == NULL )
		return 0;
	if( param_group_exists( group_name ) == 0 && make_param_group( group_name, START_GROUP_SIZE ) == 0 )
		return 0;
	while( fgets( buf, 1024, in ) != NULL ) {
		/* fprintf( stderr, "line=%s", buf ); */
		for( i= 0; buf[i] == ' ' || buf[i] == '\t'; i++ )
			;
		if( ! isalpha(buf[i]) && buf[i] != '_' )
			continue; /* go to the next line */
		key= buf+i;
		for( i++; isalpha(buf[i]) || buf[i] == '_'; i++ )
			;
		if( buf[i] != '=' && buf[i] != ' ' && buf[i] != '\t' )
			continue;
		buf[i++]= '\0';
		/* fprintf( stderr, "key='%s'", key ); */
		while( buf[i] == ' ' || buf[i] == '\t' || buf[i] == '=' )
			i++;
		if( ! isalpha(buf[i]) && buf[i] != '_' && ! isdigit(buf[i]) && buf[i] != '.' && buf[i] != '-' && buf[i] != '+' )
			continue; /* go to the next line */
		val = buf+i;
		for( i++; ! isspace(buf[i]); i++ )
			;
		buf[i++]= '\0';
		/* fprintf( stderr, " val='%s'\n", val ); */
		if( add_parameter( group_name, key, val ) != 1 ) {
			fclose( in );
			return 0;
		}
	}	
	fclose( in );
	return 1;
}

int write_param_file( char *filepath, char *group_name ) {
	int i;
	if( all_parms == NULL )
		return 0;
	for( i= 0; i < all_parms->size; i++ )
		if( strcmp( group_name, all_parms->g[i].name ) == 0 ) {
			param_group_t *g= all_parms->g+i;
			int j;
			FILE *out = fopen( filepath, "w" );
			if( out == NULL )
				return 0;
			for( j= 0; j < g->size; j++ )
					fprintf( out, "%s = %s\n", g->p[j].key, g->p[j].val );
			fclose( out );
			return 1;
		}
	return 0;
}

char *get_param( char *group, char *key ) {
	int i;
	for( i= 0; i < all_parms->size; i++ )
		if( strcmp( group, all_parms->g[i].name ) == 0 ) {
			param_group_t *g= all_parms->g+i;
			int j;
			for( j= 0; j < g->size; j++ )
				if( strcmp( g->p[j].key, key ) == 0 )
					return g->p[j].val;
			return NULL;
		}
	return NULL;
}

void free_params( void ) {
	int i;
	for( i= 0; i < all_parms->size; i++ ) {
		param_group_t *g = all_parms->g+i;
		// fprintf( stderr, "%s (%d):\n", g->name, g->size );
		int k;
		for( k= 0; k < g->size; k++ ) {
			// fprintf( stderr, "\t%s -> %s\n", g->p[k].key, g->p[k].val );
			free( g->p[k].key );
			free( g->p[k].val );
		}
		free( g->name );
		free( g->p );
	}
	free( all_parms->g );
	free( all_parms );
}
