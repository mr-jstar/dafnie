#include "dafnie.h"
#include "params.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define DEBUG_POP 0

#define BLOCK_SIZE 64

typedef enum boolean { NO, YES } boolean_t;

char *help = "Usage: %s (<opts> | -h )\n"
             "    -h - print this help and exit\n"
             "\n"
             "    <opts> = [ config-file [ stat-file ] [ log-file ] ]\n"
						 "    config-file - see example config\n"
             "    stat-file   - global statistics goes there (in csv format)\n"
						 "                  stat-file defaults to stdout\n"
             "    log-file    - program log goes there (csv format is used if file name ends with \"csv\")\n"
             "                  log is made only if a log-file is given in the argument list.\n";
#include "ver.h"

void pop_profile( int e, int s, FILE* log, population_t p ) {
	// TO DO !!!!
}

void add_strat_profile( strategy_stat_t *source, strategy_stat_t *avg ) {
	int j;
	if( source->length < 1 )
		fprintf( stderr, "\t%d-length strategy encountered in add_strat_profile!\n", source->length );
	avg->length = source->length;
	avg->born += source->born;
  avg->reproduced += source->reproduced;
  avg->ready += source->ready;
  avg->waiting += source->waiting;
	for( j= 1; j <= source->length; j++ )
		avg->avg[j] += source->avg[j];
	avg->avg[MAX_STRAT_LENGTH+1] += source->avg[MAX_STRAT_LENGTH+1];
}

void strat_profile( strategy_stat_t *s, FILE *out ) {
	int j;
	fprintf( out, "%3d: [", s->length );
	for( j= 1; j <= s->length; j++ )
		fprintf( out, "%g;", s->avg[j] );
	fprintf( out, "] mwt=%g\n", s->avg[MAX_STRAT_LENGTH+1] );
}

void plot_pop( population_t p, int strategy_length, char *out_name ) {
	FILE *out;
	out = fopen( out_name, "w" );
	if( out ) {
		fprintf( stderr, "Saving view to %s\n", out_name );
		int i,o;
		for( i= 1; i <= strategy_length; i++ ) {
			fprintf( out, "%i", i );
			for( o= 0; o < p->n; o++ )
				fprintf( out, " %i", GET(p,o)[i] );
			fprintf( out, "\n" );
		}
		fprintf( out, "%i", MAX_STRAT_LENGTH+1 );
		for( o= 0; o < p->n; o++ )
			fprintf( out, " %i", GET(p,o)[MAX_STRAT_LENGTH+1] );
		fprintf( out, "\n" );
		fclose( out );
		fprintf( stderr, "Plot saved to %s\n", out_name );
	}
}

int main( int argc, char *argv[] ) {
	int e, s, o;  /* experiment, year of experiment, organism in experiment */
	int i,j;

	int n_experiments = 10;
	int n_seasons = 2000;
	int n_organisms_at_start = 1000;
	int no_successors = 6;
	int min_strategy_length = MAX_STRAT_LENGTH / 2;
	int max_strategy_length = MAX_STRAT_LENGTH;
	int min_strategy_at_start = min_strategy_length;
	int max_strategy_at_start = max_strategy_length;
	boolean_t random_start = NO;
	double egg_entrance_survival_prob = 0.5;
	double egg_seasonal_survival_prob = 0.99;
	double organism_survival_prob = 0.5;
	double procreation_prob = 0.97;
	double mutation_prob = 0.02;
	double mutation_delta = 0.05;
	int strategy_length_delta = 1;
	int envinronment_capacity = 10000;
	int envinronment_variability = 500;
	double variability_skew = 0;

	int writelog = argc > 3;

	FILE *stat = argc > 2 && strcmp( argv[2], "-" ) ? fopen( argv[2], "w" ) : stdout;
	FILE *log = argc < 4 || strcmp( argv[3], "-" ) == 0 ? stdout : fopen( argv[3], "w" );

	int deeplog = argc > 4;

	if( argc == 2 && strcmp( argv[1], "-h" ) == 0 ) {
		printf( help, argv[0] );
		printf( "Program version: %s\n", ver );
		return 0;
	}

	int logcsv = 0;
	if( argc > 3 ) {
		char *p = strstr( argv[3], "csv" );
		if( p != NULL && strcmp( p, "csv" ) == 0 )
			logcsv= 1;
		else {
			p = strstr( argv[3], "CSV" );
			if( p != NULL && strcmp( p, "CSV" ) == 0 )
				logcsv= 1;
		}
	}

	if( argc > 1 ) {
		char *p;
		if( read_param_file( argv[1], "dafnie" ) != 1 ) {
			fprintf( stderr, "%s: bad config file %s\n", argv[0], argv[1] );
			return 1;
		}
		n_experiments = (p = get_param( "dafnie", "n_experiments" )) != NULL ? atoi( p ) : n_experiments;
		n_seasons = (p = get_param( "dafnie", "n_seasons" )) != NULL ? atoi( p ) : n_seasons;
		n_organisms_at_start = (p = get_param( "dafnie", "n_organisms_at_start" )) != NULL ? atoi( p ) : n_organisms_at_start;
		no_successors = (p = get_param( "dafnie", "no_successors" )) != NULL ? atoi( p ) : no_successors;
		min_strategy_length = (p = get_param( "dafnie", "min_strategy_length" )) != NULL ? atoi( p ) : min_strategy_length;
		max_strategy_length = (p = get_param( "dafnie", "max_strategy_length" )) != NULL ? atoi( p ) : max_strategy_length;
		min_strategy_at_start = (p = get_param( "dafnie", "min_strategy_at_start" )) != NULL ? atoi( p ) : min_strategy_at_start;
		max_strategy_at_start = (p = get_param( "dafnie", "max_strategy_at_start" )) != NULL ? atoi( p ) : max_strategy_at_start;
		random_start = (p = get_param( "dafnie", "random_start" )) != NULL ? (boolean_t)atoi( p ) : random_start;
		egg_entrance_survival_prob = (p = get_param( "dafnie", "egg_entrance_survival_prob" )) != NULL ? atof( p ) : egg_entrance_survival_prob;
		egg_seasonal_survival_prob = (p = get_param( "dafnie", "egg_seasonal_survival_prob" )) != NULL ? atof( p ) : egg_seasonal_survival_prob;
		organism_survival_prob = (p = get_param( "dafnie", "organism_survival_prob" )) != NULL ? atof( p ) : organism_survival_prob;
		procreation_prob = (p = get_param( "dafnie", "procreation_prob" )) != NULL ? atof( p ) : procreation_prob;
		mutation_prob = (p = get_param( "dafnie", "mutation_prob" )) != NULL ? atof( p ) : mutation_prob;
		mutation_delta = (p = get_param( "dafnie", "mutation_delta" )) != NULL ? atof( p ) : mutation_delta;
		strategy_length_delta = (p = get_param( "dafnie", "strategy_length_delta" )) != NULL ? atoi( p ) : strategy_length_delta;
		envinronment_capacity = (p = get_param( "dafnie", "envinronment_capacity" )) != NULL ? atoi( p ) : envinronment_capacity;
		envinronment_variability = (p = get_param( "dafnie", "envinronment_variability" )) != NULL ? atoi( p ) : envinronment_variability;
		variability_skew = (p = get_param( "dafnie", "variability_skew" )) != NULL ? atof( p ) : variability_skew;
	}
	if( min_strategy_length > max_strategy_length || min_strategy_length < 1 || max_strategy_length > MAX_STRAT_LENGTH ||
			min_strategy_at_start < min_strategy_length || max_strategy_at_start > max_strategy_length ) {
		fprintf( stderr, "Sorry: strategy length range (%d-%d) is invalid\nIf it is not an error, the code needs to be recompiled.\n", min_strategy_length, max_strategy_length );
		return 1;
	}
	if( min_strategy_at_start < min_strategy_length || max_strategy_at_start > max_strategy_length || min_strategy_at_start > max_strategy_at_start ) {
		fprintf( stderr, "Sorry: strategy at start must be inside the min-max range: %d-%d.\n", min_strategy_length, max_strategy_length );
		return 1;
	}

	time_t curtime;
	clock_t start;
	time(&curtime);
	start = clock();
	fprintf( stderr, "Started at %s\n", ctime(&curtime) );

	if( writelog ) {
		if( logcsv ) {
			fprintf( log, "%s;%d\n", "n_experiments", n_experiments );
			fprintf( log, "%s;%d\n", "n_seasons", n_seasons );
			fprintf( log, "%s;%d\n", "n_organisms_at_start", n_organisms_at_start );
			fprintf( log, "%s;%d\n", "no_successors", no_successors );
			fprintf( log, "%s;%d\n", "min_strategy_length", min_strategy_length );
			fprintf( log, "%s;%d\n", "max_strategy_length", max_strategy_length );
			fprintf( log, "%s;%d\n", "min_strategy_at_start", min_strategy_at_start );
			fprintf( log, "%s;%d\n", "max_strategy_at_start", max_strategy_at_start );
			fprintf( log, "%s;%d\n", "random_start", random_start );
			fprintf( log, "%s;%g\n", "egg_entrance_survival_prob", egg_entrance_survival_prob );
			fprintf( log, "%s;%g\n", "egg_seasonal_survival_prob", egg_seasonal_survival_prob );
			fprintf( log, "%s;%g\n", "organism_survival_prob", organism_survival_prob );
			fprintf( log, "%s;%g\n", "procreation_prob", procreation_prob );
			fprintf( log, "%s;%g\n", "mutation_prob", mutation_prob );
			fprintf( log, "%s;%g\n", "mutation_delta", mutation_delta );
			fprintf( log, "%s;%d\n", "strategy_length_delta", strategy_length_delta );
			fprintf( log, "%s;%d\n", "envinronment_capacity", envinronment_capacity );
			fprintf( log, "%s;%d\n", "envinronment_variability", envinronment_variability );
			fprintf( log, "%s;%g\n", "variability_skew", variability_skew );
		} else {
			fprintf( log, "%50s = %d\n", "n_experiments", n_experiments );
			fprintf( log, "%50s = %d\n", "n_seasons", n_seasons );
			fprintf( log, "%50s = %d\n", "n_organisms_at_start", n_organisms_at_start );
			fprintf( log, "%50s = %d\n", "no_successors", no_successors );
			fprintf( log, "%50s = %d\n", "min_strategy_length", min_strategy_length );
			fprintf( log, "%50s = %d\n", "max_strategy_length", max_strategy_length );
			fprintf( log, "%50s = %d\n", "min_strategy_at_start", min_strategy_at_start );
			fprintf( log, "%50s = %d\n", "max_strategy_at_start", max_strategy_at_start );
			fprintf( log, "%50s = %d\n", "random_start", random_start );
			fprintf( log, "%50s = %g\n", "egg_entrance_survival_prob", egg_entrance_survival_prob );
			fprintf( log, "%50s = %g\n", "egg_seasonal_survival_prob", egg_seasonal_survival_prob );
			fprintf( log, "%50s = %g\n", "organism_survival_prob", organism_survival_prob );
			fprintf( log, "%50s = %g\n", "procreation_prob", procreation_prob );
			fprintf( log, "%50s = %g\n", "mutation_prob", mutation_prob );
			fprintf( log, "%50s = %g\n", "mutation_delta", mutation_delta );
			fprintf( log, "%50s = %d\n", "strategy_length_delta", strategy_length_delta );
			fprintf( log, "%50s = %d\n", "envinronment_capacity", envinronment_capacity );
			fprintf( log, "%50s = %d\n", "envinronment_variability", envinronment_variability );
			fprintf( log, "%50s = %g\n", "variability_skew", variability_skew );
		}
	}

	srand(time(NULL));

	/* create and initialize space for averaging */
	int n_strategies= max_strategy_length-min_strategy_length+1;
	double *global_wait_time = (double*)calloc( n_seasons, sizeof * global_wait_time );
	int *global_waiting = (int*)calloc( n_seasons, sizeof * global_waiting );
	int *global_ready = (int*)calloc( n_seasons, sizeof * global_ready );
	int *global_profile = (int*)calloc( n_seasons*n_strategies, sizeof *global_profile );
	int *experiments_done = (int*)calloc( n_seasons, sizeof * experiments_done );
	strategy_stat_t *profiles = (strategy_stat_t*)calloc( n_experiments*n_strategies, sizeof *profiles );
	int *lifetime = (int*)calloc( n_experiments*n_strategies, sizeof *lifetime );
	strategy_stat_t *avg_profile = (strategy_stat_t*)calloc( n_strategies, sizeof *avg_profile );
	int *exp_extincted = (int*)calloc( n_experiments, sizeof *exp_extincted );

	/* create and initialize statistics */
	statistics_t statistics = (statistics_t)malloc( sizeof *statistics );
	strategy_stat_t *stats = (strategy_stat_t*) malloc( n_strategies*(sizeof *stats) );
	statistics->stats= stats;
	statistics->min_lgt= min_strategy_length;
	statistics->max_lgt= max_strategy_length;
	/* average population fluctuations (ready individuals) */
	double avg_fluct = 0;
  /* extinctions counter */
	int extinctions = 0;

	if( writelog ) {
		if( logcsv ) {
			fprintf( log, "experiment;season;ready;waiting;hh;env-capacity;mean-waittime;" );
			for( i=  min_strategy_length; i <= max_strategy_length; i++ )
				fprintf( log, "%d;", i );
			fprintf( log, "\n" );
		} else {
			fprintf( log, "Start\n" );
		}
	}

	/* allocate population space */
	int n_blocks = 2*n_organisms_at_start / BLOCK_SIZE;
	population_t p= make_population( (n_blocks < 1 ? 1 : n_blocks), BLOCK_SIZE );
	/* loop over experiment repetitions */
	for( e = 0; e < n_experiments; e++ ) {
		/* fill initial population */
		int strategy_length = min_strategy_at_start-1;  // it is increased at the beginning of the loop
		for( o= 0; o < n_organisms_at_start; o++ ) {
			if( strategy_length == max_strategy_at_start )
				strategy_length= min_strategy_at_start;
			else
				strategy_length++;
			if( random_start )
				fill_randomly( GET(p,o), strategy_length );
			else
				fill_uniformly( GET(p,o), strategy_length );
		}
		p->n = n_organisms_at_start;
#if DEBUG_POP
		plot_pop( p, max_strategy_at_start, "population" );
#endif
		init_statistics( statistics );
		make_statistics( p, statistics );
		statistics->surv_prob = organism_survival_prob;
		statistics->ec = envinronment_capacity;
		if( writelog ) {
			if( logcsv ) {
				fprintf( log, "%d;initial;", e );
				fprintf( log, "%d;%d;%g;%d;", statistics->ready, statistics->waiting, statistics->surv_prob, statistics->ec );
				for( i=  0; i < n_strategies; i++ )
					fprintf( log, "%d;", statistics->stats[i].ready+statistics->stats[i].waiting );
				fprintf( log, "\n" );
			} else {
				fprintf( log, "e=%3d initial: ", e );
				fprintf( log, "ready=%9d, waiting=%9d, surv.prob=%5.4f, env_cap=%9d, ", statistics->ready, statistics->waiting, statistics->surv_prob, statistics->ec );
				fprintf( log, "number=[" );
				for( i=  0; i < n_strategies; i++ )
					fprintf( log, "%d;", statistics->stats[i].ready+statistics->stats[i].waiting );
				fprintf( log, " ]\n" );
			}
		}

		/* main experiment loop: evaluate population for n_seasons */
		int prev_act = n_organisms_at_start;
		for( s= 0; s < n_seasons; s++ ) {
#ifdef SHOW_POP_PROFILE
			if( deeplog )
				pop_profile( e, s, log, p );
#endif
			if( evaluate_population( p, no_successors,
						 egg_entrance_survival_prob, egg_seasonal_survival_prob, organism_survival_prob,
						 procreation_prob, envinronment_capacity, envinronment_variability, variability_skew,
						 mutation_prob, (int)(mutation_delta*255), strategy_length_delta, min_strategy_length, max_strategy_length,
						 statistics ) == 0 ) {
				fprintf( stderr, "%s failed in %d-season of %d-experiment\n", argv[0], s, e );
				return EXIT_FAILURE;
			}
#if DEBUG_POP
			if( s == 4500 )
				plot_pop( p, max_strategy_at_start, "pop10" );
#endif
			for( i= 0; i < n_strategies; i++ ) {
				global_profile[s*(n_strategies)+i] += statistics->stats[i].ready+statistics->stats[i].waiting;
				if( statistics->stats[i].ready+statistics->stats[i].waiting > 0 )
					*(lifetime+e*n_strategies+i) += 1; // collects strategies lifetimes
		  }
			global_ready[s] += statistics->ready;
			global_waiting[s] += statistics->waiting;
			global_wait_time[s] += statistics->wait_time;
			if( writelog ) {
				if( logcsv ) {
					fprintf( log, "%d;%d;", e, s );
					fprintf( log, "%d;%d;%g;%d;", statistics->ready, statistics->waiting, statistics->surv_prob, statistics->ec );
					for( i=  0; i < n_strategies; i++ )
						fprintf( log, "%d;", statistics->stats[i].ready+statistics->stats[i].waiting );
					fprintf( log, "\n" );
				} else {
					fprintf( log, "e=%3d s=%4d: ", e, s );
					fprintf( log, "ready=%9d, waiting=%9d, surv.prob=%5.4f, env_cap=%9d, ", statistics->ready, statistics->waiting, statistics->surv_prob, statistics->ec );
          fprintf( log, "mean wait time=%9g, ", statistics->wait_time );
					fprintf( log, "number=[" );
					for( i= 0; i < n_strategies; i++ )
						fprintf( log, " %d", statistics->stats[i].ready+statistics->stats[i].waiting );
					fprintf( log, " ]\n" );
          /*
					fprintf( log, "wait-time=[" );
					for( i= 0; i < n_strategies; i++ )
						fprintf( log, " %g", statistics->stats[i].avg[MAX_STRAT_LENGTH+1] );
					fprintf( log, " ]\n" );
          */
				}
			}
			if( s > 0 ) {
				avg_fluct += (prev_act - statistics->ready)*(prev_act - statistics->ready);
				prev_act = statistics->ready;
				if( statistics->ready+statistics->waiting == 0 ) {
					extinctions++;
					exp_extincted[e] = 1;
					break;
				}
			}
			experiments_done[s]++;
		} // end for( s...) loop - seasons
		if( statistics->ready+statistics->waiting > 0 ) {
			//for( i= 0; i < n_strategies; i++ )   // diagnostyka
			//	strat_profile( statistics->stats+i, log );
			for( i= 0; i < n_strategies; i++ )
				if( statistics->stats[i].ready+statistics->stats[i].waiting == 0 )
					(profiles+e*n_strategies+i)->no_extinctions= 1;
				else {
					add_strat_profile( statistics->stats+i, profiles+e*n_strategies+i );
//fprintf( stderr, "Exp: %d, strat %d, length = %d\n", e, i, (profiles+e*n_strategies+i)->length );
				}
		} else {
			for( i= 0; i < n_strategies; i++ )
				(profiles+e*n_strategies+i)->no_extinctions= 1;
		}
		
	} // end for( e ... ) loop - experiments
	if( writelog ) fclose( log );

	fprintf( stat, "%s;%d\n", "n_experiments", n_experiments );
	fprintf( stat, "%s;%d\n", "n_seasons", n_seasons );
	fprintf( stat, "%s;%d\n", "n_organisms_at_start", n_organisms_at_start );
	fprintf( stat, "%s;%d\n", "no_successors", no_successors );
	fprintf( stat, "%s;%d\n", "min_strategy_length", min_strategy_length );
	fprintf( stat, "%s;%d\n", "max_strategy_length", max_strategy_length );
	fprintf( stat, "%s;%d\n", "min_strategy_at_start", min_strategy_at_start );
	fprintf( stat, "%s;%d\n", "max_strategy_at_start", max_strategy_at_start );
	fprintf( stat, "%s;%d\n", "random_start", random_start );
	fprintf( stat, "%s;%g\n", "egg_entrance_survival_prob", egg_entrance_survival_prob );
	fprintf( stat, "%s;%g\n", "egg_seasonal_survival_prob", egg_seasonal_survival_prob );
	fprintf( stat, "%s;%g\n", "organism_survival_prob", organism_survival_prob );
	fprintf( stat, "%s;%g\n", "procreation_prob", procreation_prob );
	fprintf( stat, "%s;%g\n", "mutation_prob", mutation_prob );
	fprintf( stat, "%s;%g\n", "mutation_delta", mutation_delta );
	fprintf( stat, "%s;%d\n", "strategy_length_delta", strategy_length_delta );
	fprintf( stat, "%s;%d\n", "envinronment_capacity", envinronment_capacity );
	fprintf( stat, "%s;%d\n", "envinronment_variability", envinronment_variability );
	fprintf( stat, "%s;%g\n", "variability_skew", variability_skew );
	fprintf( stat, "ready;waiting;mean-wait-time;" );
	for( i= 0; i < n_strategies; i++ )
		fprintf( stat, "%d;", i+statistics->min_lgt );
	fprintf( stat, "\n" );
	int all_seasons = 0;
	double avg_final_size;
	for( s= 0; s < n_seasons; s++ ) {
		avg_final_size = 0;
		if( experiments_done[s] > 0 ) {
			fprintf( stat, "%d;%d;", global_ready[s] / experiments_done[s], global_waiting[s] / experiments_done[s] );
      fprintf( stat, "%g;", global_wait_time[s] / experiments_done[s]  ); // wpisać sredni czas DP
			for( i= 0; i < n_strategies; i++ ) {
				fprintf( stat, "%g;", (double)global_profile[s*(n_strategies)+i] / experiments_done[s] );
				avg_final_size += (double)global_profile[s*(n_strategies)+i] / experiments_done[s];
			}
		} else {
			fprintf( stat,"0;0;" );
			for( i= 0; i < n_strategies; i++ )
				fprintf( stat, "0;" );
		}
		fprintf( stat, "\n" );
		all_seasons += experiments_done[s];
	}
	fprintf( stat, "average profiles for strategies;\n" );
	double weight_denom = 0.0;
	double weights[MAX_STRAT_LENGTH];
	for( i= 0; i < n_strategies; i++ ) {
		double stddev[MAX_STRAT_LENGTH+1];
		memset( stddev, 0, (MAX_STRAT_LENGTH+1)*sizeof stddev[0] );
		int this_stat_extinctions = 0;
		for( e= 0; e < n_experiments; e++ ) {
			if( (profiles+e*n_strategies+i)->no_extinctions == 1 )
				this_stat_extinctions++;
			else {
//fprintf( stderr, "Strat %d, adding exp. %d to average\n", i, e );
				add_strat_profile( profiles+e*n_strategies+i, avg_profile+i );
			}
		}
		avg_profile[i].no_extinctions =  this_stat_extinctions;
//fprintf( stderr, "Stat %i - extinctions = %i\n", i, this_stat_extinctions );
		weights[i] = 0;
		if( this_stat_extinctions < n_experiments ) {
			weights[i] = (avg_profile[i].ready+avg_profile[i].waiting) / n_experiments;
			weight_denom += weights[i];
			avg_profile[i].born /=  n_experiments - this_stat_extinctions;
			avg_profile[i].reproduced /=  n_experiments - this_stat_extinctions;
			avg_profile[i].ready /=  n_experiments - this_stat_extinctions;
			avg_profile[i].waiting /=  n_experiments - this_stat_extinctions;
			for( j= 1; j <= avg_profile[i].length; j++ ) {
				avg_profile[i].avg[j] /=  n_experiments - this_stat_extinctions;
				stddev[j-1]= 0;
				for( e= 0; e < n_experiments; e++ ) {
					if( ! exp_extincted[e] ) {
						double d= (profiles+e*n_strategies+i)->avg[j] - avg_profile[i].avg[j];
						stddev[j-1] += d*d;
					}
				}
				stddev[j-1] /= (n_experiments - this_stat_extinctions)*(n_experiments - this_stat_extinctions-1);
				stddev[j-1] = sqrt( stddev[j-1] );
	    }
			avg_profile[i].avg[MAX_STRAT_LENGTH+1] /=  n_experiments - this_stat_extinctions;
			stddev[MAX_STRAT_LENGTH+1] = 0;
			if( n_experiments - this_stat_extinctions > 1 ) {
				for( e= 0; e < n_experiments; e++ )
					if( ! exp_extincted[e] ) {
						double d = (profiles+e*n_strategies+i)->avg[MAX_STRAT_LENGTH+1] - avg_profile[i].avg[MAX_STRAT_LENGTH+1];	
						stddev[MAX_STRAT_LENGTH+1] += d*d;
					}
				stddev[MAX_STRAT_LENGTH+1] /= (n_experiments - this_stat_extinctions)*(n_experiments - this_stat_extinctions-1);
				stddev[MAX_STRAT_LENGTH+1] = sqrt( stddev[MAX_STRAT_LENGTH+1] );
      	fprintf( stat, "strat;%d;", avg_profile[i].length );
      	for( j= 1; j <= avg_profile[i].length; j++ )
					fprintf( stat, "%g;", avg_profile[i].avg[j] );
				fprintf( stat, "mwt;%g;\n", avg_profile[i].avg[MAX_STRAT_LENGTH+1] );
				fprintf( stat, "U(t),(k=2);%d;", avg_profile[i].length );
				for( j= 1; j <= avg_profile[i].length; j++ )
      		fprintf( stat, "%g;", 2*stddev[j-1] );
				fprintf( stat, "U(mwt),(k=2);%g;\n", stddev[MAX_STRAT_LENGTH+1] );
			} else {
				fprintf( stat, "single survivorship for strategy %d\n;", avg_profile[i].length );
			}
		} else {	
      fprintf( stat, "strat extincted in all experiments;\n" );
		}
	}

	double *strat_weight = (double *) calloc( n_strategies, sizeof *strat_weight );
	double *extncts_percent = (double *) calloc( n_strategies, sizeof *extncts_percent );
	double *lifetime_avg = (double *) calloc( n_strategies, sizeof *lifetime_avg );
	double *lifetime_stdv = (double *) calloc (n_strategies, sizeof *lifetime_stdv );
	fprintf( stat, "weighted average profiles for strategies;\n" );
	for( i= 0; i < n_strategies; i++ ) {
		strat_weight[i] = weights[i]/weight_denom;
    fprintf( stat, "strat;%d;", avg_profile[i].length );
    for( j= 1; j <= avg_profile[i].length; j++ )
			fprintf( stat, "%g;", avg_profile[i].avg[j]*strat_weight[i] );
		for( j= avg_profile[i].length+1; j <= max_strategy_length; j++ )
			fprintf( stat, ";" );
		extncts_percent[i] = 100.*avg_profile[i].no_extinctions/n_experiments;
		fprintf( stat, "weight;%g;%% extinctions;%g\n", strat_weight[i], extncts_percent[i] );
	}
	for( i= 0; i < n_strategies; i++ ) {
		double mean_lifetime= 0.0;
		double stdev= 0.0;
		for( e= 0; e < n_experiments; e++ )
			mean_lifetime += *(lifetime+e*n_strategies+i);
		mean_lifetime /= n_experiments;
		for( e= 0; e < n_experiments; e++ )
			stdev += (*(lifetime+e*n_strategies+i)-mean_lifetime)*(*(lifetime+e*n_strategies+i)-mean_lifetime);
		stdev /= n_experiments*(n_experiments-1);
		stdev = sqrt(stdev);
		lifetime_avg[i] = mean_lifetime;
		lifetime_stdv[i] = stdev;
	}

	fprintf( stat, "weights for strategies;" );
	fprintf( stat, "survivorship;" );
	fprintf( stat, "average lifetime for strategies;" );
	fprintf( stat, "std.dev. of the lifetime for strategies;" );
	fprintf( stat, "profile of the last population;\n" );
	for( i = 0; i < n_strategies; i++ ) {
		fprintf( stat, "%g;", strat_weight[i] );
		fprintf( stat, "%g;", 100-extncts_percent[i] );
		fprintf( stat, "%g;", lifetime_avg[i] );
		fprintf( stat, "%g;", lifetime_stdv[i] );
		fprintf( stat, "%g;", (double)global_profile[(n_seasons-1)*(n_strategies)+i] / experiments_done[n_seasons-1] / avg_final_size );
	  fprintf( stat, "\n" );
  }

	fprintf( stat, "avg. fluct;%g;\n", sqrt(avg_fluct) / all_seasons );
	fprintf( stat, "extinctions;%d;\n", extinctions );

	fclose( stat );

	free( strat_weight );
	free( extncts_percent );
	free( lifetime_stdv );
	free( lifetime_avg );
  free( global_waiting );
  free( global_ready );
  free( global_profile );
	free( experiments_done );
  free( statistics );
	free( avg_profile );
  free( profiles );
	free( exp_extincted );
	free_population( p );
	free_params( );

  time(&curtime);
	start = clock() - start;
  fprintf( stderr, "Ended at   %s\n", ctime(&curtime) );
	if( (double)(start/CLOCKS_PER_SEC) > 3600 ) {
		int h = start/CLOCKS_PER_SEC/3600;
		int m = start/CLOCKS_PER_SEC%3600/60;
		float s = start/CLOCKS_PER_SEC%60;
		fprintf( stderr, "Elapsed time: %d:%d:%g [h:m:s]\n", h, m, s );
	} else if( (double)(start/CLOCKS_PER_SEC) > 60 ) {
		int m = start/CLOCKS_PER_SEC/60;
		float s = start/CLOCKS_PER_SEC%60;
		fprintf( stderr, "Elapsed time: %d:%g [m:s]\n", m, s );
	} else {
		fprintf( stderr, "Elapsed time: %g [s]\n", (double)(start/CLOCKS_PER_SEC) );
	}

	return 0;
}
