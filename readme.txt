The program simulates competition between organisms (showing various life strategies
   that differ in diapause length distribution) competing for limited resources
   that vary stochastically from generation to generation.

The project contains the following files:
(if you are not a computer-specialist go directly to number 3)

1) *.c and *.h are source files of the application.

2) Makefile for gnumake under Linux od Unix. It will buld an executable eldevol (the default target)
   or a tester for skeved random number generator. To test the ececutable run 'make test'.

3) ELDevol324.exe is Windows executable file that simulates competition betwen organisms.
   The program requires a configuration file with the tested parameters.
   The program saves results in two text (CSV) files:
    - the first (more usefull) contains contains mean values and statistics calculated upon
      the raw data in the second file.
    - the second file stores raw results of the simulations (generation by generation:

   To make simulation, typically place the simulation file ELDevol324.exe in the same directory as the config file.

   -----------------------------------------------

   The syntax to run the simulation under Windows in the console:

      ELDevol324.exe config324.txt name_of_the_file_containing_statistics.csv name_of_the_raw_data_file.csv

   -----------------------------------------------

4) config324.txt contains the following parameters used during simulations:

n_experiments                 = 1000 # the number of experiments to run
n_seasons                     = 5000 # the number of generations during each run
n_organisms_at_start          = 1000 # initial population size
no_successors                 = 6 # fecundity of the tested organisms
min_strategy_length           = 1 # min possible length of offspring dormancy, where 1 means no dormancy 
max_strategy_length           = 50 # max possible length of offspring dormancy (50 or lower values)
min_strategy_at_start         = 1 # min initial length of offspring dormancy
max_strategy_at_start         = 50 # max initial length of dormancy
random_start                  = 1     # initial distribution of offspring length dormancy with 2 possible values 1=random distribution, 0=even distribution 
egg_entrance_survival_prob    = 0.95 # survival probability of the dormant eggs during the first season after formation - settlement survival probability
egg_seasonal_survival_prob    = 0.95 # survival probability of each following season of the dormant egg
organism_survival_prob        = 1 # survival probability of the active offspring till reproduction at nonlimiting food conditions
procreation_prob              = 1 # procreation probability at nonlimiting food conditions
mutation_prob                 = 0.00001 # mutation probability of the dormancy parameters (min, max length of diapause)
mutation_delta                = 0.02 
strategy_length_delta         = 1 # step of possible mutations
envinronment_capacity         = 500 # expected value of environmental capacity
envinronment_variability      = 500  # value of 1 standard deviation of environmental variability
variability_skew              = 0    # skew of env. capacity distribution, where 0 means symetric distribution

5) readme file  - the information file you are reading presently

