#include "dafnie.h"
#include "myrandom.h"
#include <string.h>
#include <stdio.h>

// IS_DEAD must be > MAX_STRAT_LENGTH (defined in dafnie.h)
#define IS_DEAD 113

population_t make_population( size_t n_blocks, size_t block_size ) {
// allocate space for n_blocks * block_size population
	population_t p = (population_t)malloc( sizeof *p );
	if( p == NULL )
		return NULL;

	if( (p->b = (block_t*)malloc( n_blocks * sizeof * p->b )) == NULL ) {
		free( p );
		return NULL;
	}
	int i;
	for( i= 0; i < n_blocks; i++ )
		if( (p->b[i] = (block_t)malloc( block_size * sizeof * p->b[0] )) == NULL ) {
			int j;
			for( j= 0; j < i; j++ )
				free( p->b[j] );
			free( p->b );
			free( p );	
			return NULL;
		}
	p->n_blocks = n_blocks;
	p->block_size = block_size;
	p->n = 0;
	return p;
}

int resize_population( population_t p, int plus_blocks ) {
// resize population by plus_blocks * block_size
	block_t *nb = (block_t*)realloc( p->b, (p->n_blocks+plus_blocks)*sizeof * p->b );
	if( nb == NULL )
		return 0;
	/*memset( p->n_blocks*sizeof *p->b, 0, plus_blocks*sizeof *p->b ); */
	p->b = nb;
	int i;
	for( i= 0; i < plus_blocks; i++ )
		if( (p->b[p->n_blocks+i] = (block_t)malloc( p->block_size * sizeof * p->b[0] )) == NULL )
			return 0;
	p->n_blocks += i;
	//fprintf( stderr, ">>>>>>>>>>>>>>>> n=%d Resized by %d blocks to %d x %d\n        ", p->n, plus_blocks, p->n_blocks, p->block_size );
	return i;
}
		
void free_population( population_t p ) {
// free memory ocupied by population
	int i;
	for( i= 0; i < p->n_blocks; i++ )
		free( p->b[i] );
	free( p->b );
	free( p );
}

int add_organism( population_t p, organism_t o ) {
	int plus_blocks = p->n_blocks / 2 > 0 ? p->n_blocks / 2 : 1;
#if 0
	if( ! is_valid( o ) ) {
		fprintf( stderr, "INVALID ORGANISM in add_organism: " );
		print_organism( o, stderr );
		fprintf( stderr, "\n" );
	}
#endif
	if( NO_FREE_SPACE(p) && resize_population( p, plus_blocks ) == 0 ) {
		fprintf( stderr, "Can't resize: %d in %zd blocks of %zd organisms\n", p->n, p->n_blocks, p->block_size );
		return 0;
	} else {
		int block_no = p->n / p->block_size;
		int offset = p->n % p->block_size;
		memcpy( p->b[block_no][offset], o, ORGANISM_SIZE );
		p->n++;
		return 1;
	}
}

int kill_organism( population_t p, organism_t o, int i ) {
	if( i < p->n && p->n > 0 ) {
		int block_no = i / p->block_size;
		int offset = i % p->block_size;
		memcpy( o, p->b[block_no][offset], ORGANISM_SIZE );
		p->b[block_no][offset][0] = IS_DEAD;
		return 1;
	} else
		return 0;
}

void clean_pop( population_t p ) {
	int i,j;
	//fprintf( stderr, "clean: %d -> ", p->n );
	for( i= j= 0; i < p->n; i++ ) {
		if( *GET(p,i) != IS_DEAD ) {
			memmove( GET(p,j), GET(p,i), ORGANISM_SIZE );
			j++;
		}
	}
	p->n = j;
	//fprintf( stderr, "%d\n", p->n );
}

unsigned char * get_organism( population_t p, int i ) {
	if( i < p->n && p->n > 0 ) {
		int block_no = i / p->block_size;
		int offset = i % p->block_size;
		return p->b[block_no][offset];
	} else
		return NULL;
}

void fill_uniformly( organism_t o, unsigned int strategy_lgt ) {
	int i;
	int mean = 255 / strategy_lgt;
	int rest = 255 % strategy_lgt;
	memset( o+1, 0, MAX_STRAT_LENGTH );
	o[0] = strategy_lgt;
	memset( o+1, mean, strategy_lgt );
	for( i= 1; i <= rest; i++ )
		o[i] += 1;
	set_wait_time( o );
}

void fill_randomly( organism_t o, unsigned int strategy_lgt ) {
	int i,d,dmax;
	int rest= 255;
	memset( o+1, 0, MAX_STRAT_LENGTH );
  o[0] = strategy_lgt;
	while( rest > 0 ) {
		i = rand() % strategy_lgt + 1;
		if( 255-o[i] >= rest ) {
      dmax = rest / strategy_lgt;
    } else {
      d = (255-o[i]) / strategy_lgt;
		}
		if( dmax < 1 )
			dmax= 1;
		d = rand() % dmax + 1;
		rest -= d;
		o[i] += d;
	}
	set_wait_time( o );
	//print_organism( o, stderr );
}

void copy_mutate( organism_t s, organism_t d, int delta, int length_delta, int min_length, int max_length ){
  unsigned int old_length = s[0];
	int d_length = rand() % ( length_delta + 1 );
	unsigned int strategy_lgt = old_length + (rand() % 2 ? -d_length : d_length); // mutation can lenghten or shorten the length
	strategy_lgt = strategy_lgt < min_length ? min_length : strategy_lgt;
	strategy_lgt = strategy_lgt > max_length ? max_length : strategy_lgt;
	//fprintf( stderr, "min=%d old=%d new=%d max=%d\n", min_length, old_length, strategy_lgt, max_length );
	int decreased = rand() % old_length + 1;
	int increased = rand() % strategy_lgt + 1;
	memset( d+1, 0, MAX_STRAT_LENGTH );
	memcpy( d, s, old_length+1 );
	d[0] = strategy_lgt;
	if( decreased != increased )  {
		if( d[decreased] < delta ) {
			delta = d[decreased];
    }
		if( 255-d[increased] < delta ) {
			delta = 255-d[increased];
		}
		d[decreased] -= delta;
		d[increased] += delta;
	}
	int i, dd;
	//print_organism( s, stderr );
	//print_organism( d, stderr );
	delta = 255;
	for( i= 1; i <= strategy_lgt; i++ )
		delta -= d[i];
	while( delta > 0 ) {
		//fprintf( stderr, "delta=%d\n", delta );
		increased = rand() % strategy_lgt + 1;
		dd = rand() % ( delta + 1 );
		d[increased] += dd;
	  //fprintf( stderr, "d[%d] += %d\n", increased, dd );	
		delta -= dd;
	}
	set_wait_time( d );
	if( ! is_valid( d ) ) {
		fprintf( stderr, "copy_mutate:\n");
		print_organism( s, stderr );
		print_organism( d, stderr );
		fprintf( stderr, "\n" );
		exit(1);
	}
}

void make_copy( organism_t s, organism_t d ){
  unsigned int old_length = s[0];
	memset( d+1, 0, MAX_STRAT_LENGTH );
	memcpy( d, s, old_length+1 );
	set_wait_time( d );
	if( ! is_valid( d ) ) {
		fprintf( stderr, "make_copy: ");
		print_organism( d, stderr );
		fprintf( stderr, "\n" );
	}
}

void set_wait_time( organism_t o ) {
	unsigned int strategy_lgt = o[0];
	int ttw = rand() % 256;
	int i;
	for( i= 1; i <= strategy_lgt; i++ ) {
		if( o[i] > ttw ) {
			o[MAX_STRAT_LENGTH+1]= i-1;
			goto CHECK;
		} else {
			ttw -= o[i];
		}
 	}
	o[MAX_STRAT_LENGTH+1]= strategy_lgt-1;
CHECK:
	if( ! is_valid( o ) ) {
		fprintf( stderr, "INVALID ORGANISM in set_wait_time: " );
		print_organism( o, stderr );
		fprintf( stderr, "\n" );
	} else {
#if 0  // for testing
		print_organism( o, stdout );
#endif
  }
}

int is_valid( organism_t o ) {
	unsigned int strategy_lgt = o[0];
	int sum= 0;
	int i;
	if( strategy_lgt > MAX_STRAT_LENGTH )
		return 0;
	for( i= 1; i <= strategy_lgt; i++ )
		sum += o[i];

	return sum == 255;
}

void print_organism( organism_t o, FILE *out ) {
	int i;
	unsigned int strategy_lgt= o[0];
	fprintf( out, "(wait=%2d) profile=[", o[MAX_STRAT_LENGTH+1] );
	int s= 0;
	for( i= 1; i <= strategy_lgt; i++ ) {
		fprintf( out, " %3d", o[i] );
		s += o[i];
	}
	fprintf( out, "] sum=%d\n", s );
}

#define RAND01 ((double)rand()/RAND_MAX)

#include <math.h>

void init_statistics( statistics_t stat ) {
	int i,j;
	for( i= 0; i <= stat->max_lgt-stat->min_lgt; i++ ) {
		stat->stats[i].length = i + stat->min_lgt;
		stat->stats[i].born = 0;
		stat->stats[i].reproduced = 0;
		stat->stats[i].ready = 0;
		stat->stats[i].waiting = 0;
		for( j= 0; j < ORGANISM_SIZE; j++ )
			stat->stats[i].avg[j]= 0.0;
	}
	stat->wait_time = 0.0;
}

int evaluate_population( population_t p, int no_successors,
                         double egg_entrance_survival_prob, double egg_seasonal_survival_prob, double organism_survival_prob, double procreation_prob,
                         int expected_capacity, int envinronment_variability, double variability_skew,
                         double mutation_prob, int mutation_delta, int strategy_length_delta, int min_strategy_length, int max_strategy_length,
			 statistics_t stat
                       )
{
/* evaluates population - single season 
   returns 1 in most cases, 0 when memory allocation problems were encountered */
	int i,j;
	int envinronment_capacity;
  /* count number of organism which will activate */
	init_statistics( stat );
	int tobeborn= 0;
	for( i= 0; i < p->n; i++ ) { /* loop over organisms */
		if( (GET(p,i))[MAX_STRAT_LENGTH+1] == 0 ) {
			tobeborn++;
		}
	}
	/* calculate current capacity of envinronment */
	envinronment_capacity = (int)randSkew( expected_capacity, envinronment_variability, variability_skew );
	stat->ec = envinronment_capacity;
	/* scale organism_survival_prob by the capacity of envinronment */
	/* original formula: organism_survival_prob *= ((double)envinronment_capacity - tobeborn) / envinronment_capacity; */
	if( tobeborn > envinronment_capacity )
		organism_survival_prob *= (double)envinronment_capacity / tobeborn;
	stat->surv_prob = organism_survival_prob;
	int parent_n= p->n;
	for( i= 0; i < parent_n; i++ ) { /* loop over organisms */
		//fprintf( stderr, "\b\b\b\b\b\b\b\b%8d", i );
#ifdef VERY_DEEP_LOG
		fprintf( stderr, "%d", i );
#endif
		if( GET(p,i)[MAX_STRAT_LENGTH+1] > 0 )
			if( RAND01 < egg_seasonal_survival_prob ) {
				GET(p,i)[MAX_STRAT_LENGTH+1]--;
#ifdef VERY_DEEP_LOG
				fprintf( stderr, "-" );
#endif
			} else {
				organism_t o;
      	kill_organism( p, o, i );
#ifdef VERY_DEEP_LOG
				fprintf( stderr, "x" );
#endif
			}
		else {
			organism_t parent;
     	kill_organism( p, parent, i );
			int lgt= parent[0];
			stat->stats[lgt-stat->min_lgt].born++;
			if(  RAND01 < organism_survival_prob ) {
				if( RAND01 < procreation_prob ) {
					stat->stats[lgt-stat->min_lgt].reproduced++;
#ifdef VERY_DEEP_LOG
					fprintf( stderr, "p" );
#endif
					for( j= 0; j < no_successors; j++ ) {
						organism_t child;
						if( RAND01 < mutation_prob )
							copy_mutate( parent, child, mutation_delta, strategy_length_delta, min_strategy_length, max_strategy_length ); /* mutation */
						else
							make_copy( parent, child ); /* no mutation */
						if( child[0] == 0 || RAND01 < egg_entrance_survival_prob ) {
#ifdef VERY_DEEP_LOG
							fprintf( stderr, "." );
#endif
						if( ! is_valid( child ) ) {
    						fprintf( stderr, "INVALID ORGANISM in evaluate_population: " );
    						print_organism( child, stderr );
    						fprintf( stderr, "\n" );
  						}

						if( add_organism( p, child ) != 1 )
							return 0; /* memory problems? */
						}
					}
				} else {
#ifdef VERY_DEEP_LOG
					fprintf( stderr, "s" );
#endif
				}
			}
		}
#ifdef VERY_DEEP_LOG
		fprintf( stderr, " " );
#endif
	}
	clean_pop( p );
#ifdef VERY_DEEP_LOG
	fprintf( stderr, "\n" );
#endif
	make_statistics( p, stat );
	return 1;
}

void make_statistics( population_t p, statistics_t stat ) {
	int i,j;
	stat->waiting= 0;
	stat->ready= 0;
	for( i= 0; i < p->n; i++ ) {
		unsigned char *o = get_organism( p, i );
		unsigned int lgt= *o;
    if( lgt < stat->min_lgt || lgt> stat->max_lgt ) {
    	fprintf( stderr, "organism # %d, strategy length = %d is outside limits (%d-%d)\n", i, lgt, stat->min_lgt, stat->max_lgt );
			print_organism( o, stdout );
		}
		if( o[MAX_STRAT_LENGTH+1] > lgt )
    	fprintf( stderr, "i= %d, to long wait time =%d, but strategy length is %d\n", i, o[MAX_STRAT_LENGTH+1], lgt );
		for( j= 1; j <= lgt; j++ )
			stat->stats[lgt - stat->min_lgt].avg[j] += o[j];
		stat->stats[lgt - stat->min_lgt].avg[MAX_STRAT_LENGTH+1] += o[MAX_STRAT_LENGTH+1];
		if( o[MAX_STRAT_LENGTH+1] == 0 ) {
			stat->stats[lgt-stat->min_lgt].ready++;
			stat->ready++;
		} else {
			stat->stats[lgt-stat->min_lgt].waiting++;
			stat->waiting++;
		}
	}

	for( i= 0; i <= stat->max_lgt-stat->min_lgt; i++ ) {
		strategy_stat_t *st= &(stat->stats[i]);
		if( st->ready+st->waiting > 0 ) {
			stat->wait_time += st->avg[MAX_STRAT_LENGTH+1];
			st->avg[MAX_STRAT_LENGTH+1] /= st->ready+st->waiting;
//fprintf( stderr, "lgt=%d: waiting=%d ready=%d, mean wait time=%g\n", i+stat->min_lgt, st->ready, st->waiting, st->avg[MAX_STRAT_LENGTH+1] );
			double s= 0;
			for( j= 1; j <= stat->min_lgt+i; j++ ) {
				st->avg[j] /= (st->ready+st->waiting)*255.;
				s+= st->avg[j];
			}
			if( s < 0.999 || s > 1.001 ) {
				fprintf( stderr, "ERROR: for strategy of lgt= %d => avg sum = %.9g [ ", stat->min_lgt+i, s );
    		for( j= 1; j <= stat->min_lgt+i; j++ )
       		fprintf( stderr, "%g ", st->avg[j] );
     		fprintf( stderr, "]\n" );
				for( i= 0; i < p->n; i++ ) {
   				unsigned char *o = get_organism( p, i );
					if( !is_valid( o ) ) {
         		fprintf( stderr, "INVALID ORGANISM in make_statistics o# %6d:", i );
						print_organism( o, stderr );
         		fprintf( stderr, "\n" );
					}
				}
			}
		}
	}
	if( stat->waiting+stat->ready > 0 ) 
		stat->wait_time /= (stat->waiting+stat->ready);
}
