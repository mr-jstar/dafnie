#ifndef DAFNIE_H
#define DAFNIE_H

#include <stdlib.h>
#include <stdio.h>

#define MAX_STRAT_LENGTH 50
#define ORGANISM_SIZE MAX_STRAT_LENGTH+2  /* actual length is kept in the first (0) element, time to wait is kept in the last (MAX_STRAT_LENGHT+1) */

typedef unsigned char organism_t[ORGANISM_SIZE];   /* actual length, vector of diapause length probabilities, time to wait */

typedef organism_t *block_t;  /* block is a vector of organisms */

typedef struct pop {
	block_t *b;
	size_t n_blocks;
	size_t block_size;
	int n;
} *population_t;

typedef struct strategy_stat { // statistics for single (given length) strategy
	int length;
	int born;
	int reproduced;
	int ready;
	int waiting;
	int no_extinctions;
	double avg[ORGANISM_SIZE]; // average probability of diapause length profile
                             // first element is not used, average waiting time is stored in the last element
} strategy_stat_t;

typedef struct  stat {
	int min_lgt;  // min length of a strategy
	int max_lgt;  // max length
	int ready;
  int waiting;
  double wait_time;  // mean for all
  strategy_stat_t *stats; // statistics for strategies (this is an array of length max_lgt-min_lgt+1)
	int ec;  // envinronment capacity
	double surv_prob; // probability of survival (calculated for ec)
} *statistics_t;

#define NO_FREE_SPACE(p) ((p)->n == (p)->n_blocks*(p)->block_size)

population_t make_population( size_t n_blocks, size_t block_size );

int resize_population( population_t p, int plus_blocks );

void free_population( population_t p );

int add_organism( population_t p, organism_t o );

int kill_organism( population_t p, organism_t o, int i );

unsigned char *get_organism( population_t p, int i );

/* fast version */
#define GET(p,i) ((p)->b[(i)/(p)->block_size][(i)%(p)->block_size])

void fill_uniformly( organism_t o, unsigned int strategy_lgt );

void fill_randomly( organism_t o, unsigned int strategy_lgt );

void copy_mutate( organism_t s, organism_t d, int delta, int l_delta, int min_l, int max_l );

void make_copy( organism_t s, organism_t d );

int is_valid( organism_t o );

void print_organism( organism_t o, FILE *out );

void set_wait_time( organism_t o );

void init_statistics( statistics_t stat );

void make_statistics( population_t p, statistics_t stat );

int evaluate_population( population_t p, int no_successors,
                         double egg_entrance_survival_prob, double egg_seasonal_survival_prob, double organism_survival_prob, double procreation_prob,
                         int expected_capacity, int envinronment_variability, double variability_skew,
                         double mutation_prob, int mutattion_delta, int l_delta, int min_l, int max_l,
			 statistics_t stat );
#endif
