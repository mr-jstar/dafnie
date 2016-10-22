#include "myrandom.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <float.h>
 
#define MAX_WIDTH 50
 
/* Arguments: ./a.out mu scale, skew, samples bins [min] [max] */
 
int
main (int argc, char *argv[])
{
  int samples, bins;
  double *range, mu = 0.0, scale = 1.0, skew = 0.0, min, max, current_random_no;
  int i, j, *bin_count, *abs_bin_count, count = 0, bin, abs_bin, max_bin_count = INT_MIN, max_abs_bin_count = INT_MIN, flag = 0,zero_bin;

  double sum;
  double abs_sum;
 
  if (argc < 6)
    {
      printf ("Usage: %s mu scale skew samples bins [min] [max]\n", argv[0]);
      return 0;
    }
 
  srand (time (NULL));
 
  mu = atof (argv[1]);
  scale = atof (argv[2]);
  skew = atof (argv[3]);
  samples = atoi (argv[4]);
  bins = atoi (argv[5]);
 
  /* Allocate memory */
  bin_count = (int*)calloc (sizeof (int), bins);
  abs_bin_count = (int*)calloc (sizeof (int), bins);
  range = (double*)malloc (sizeof (double) * (bins + 1));
 
  /* Automatically set a min and max range */
  min = mu - (5-skew) * scale;
  max = mu + (5+skew) * scale;
 
  /* If min and max is specified, use them */
  if (argc >= 7)
    {
      min = atof (argv[6]);
    }
  if (argc >= 8)
    {
      max = atof (argv[7]);
    }
 
  /* Generate the bin ranges */
  range[0] = min;
  zero_bin = 0;
  for (i = 1; i <= bins; i++)
    {
      range[i] = range[0] + (max - min) * (1.0 / bins) * i;
      if( fabs(range[i]) < DBL_EPSILON ) zero_bin = i;
    }

  sum= 0; 
  abs_sum = 0;
  for (count = 0; count < samples; count++)
    {
      /* Generate random numbers from a distribution, Normal for this */
      current_random_no = randSkew (mu, scale, skew);
      sum += current_random_no;
      abs_sum += current_random_no >= 0 ? current_random_no : 0;
 
      /* Check which range the current sample falls */
      for (i = 0, bin = -1; i < bins; i++)
        {
          if ((current_random_no >= range[i])
              && (current_random_no < range[i + 1]))
            {
              bin = i;
              break;
            }
        }
      /* In case we have the number exactly equalto range[bins] */
      if (current_random_no == range[i])
        {
          bin = i - 1;
        }
      abs_bin = current_random_no >= 0 ? bin : zero_bin;
      if ((bin <= bins) && (bin >= 0))
        {
          bin_count[bin]++;
          abs_bin_count[abs_bin]++;
        }
      /* Not all the random numbers were generated within the [min,max] range. */
      else
        {
          flag = 1;
        }
    }
 
  /* Find the max value of the bin counters. This is used to scale the histogram */
  for (i = 0; i < bins; i++)
    {
      if (bin_count[i] > max_bin_count)
        {
          max_bin_count = bin_count[i];
        }
      if (abs_bin_count[i] > max_abs_bin_count)
        {
          max_abs_bin_count = abs_bin_count[i];
        }
    }
 
 
  /* Print histogram and ranges */
  printf ("[bin_low, bin_high), count\n");
  for (i = 0; i < bins; i++)
    {
      printf ("[%+7.1f,%+7.1f), %6d:", range[i], range[i + 1], bin_count[i]);
      for (j = 0;
           j < (bin_count[i] / (double) max_bin_count) * MAX_WIDTH;
           //j < (bin_count[i] / (double) max_bin_count) * (MAX_WIDTH / std);
           j++)
        {
          printf ("*");
        }
      printf ("\n");
 
    }
 
  /* Print histogram and ranges */
  printf ("[bin_low, bin_high), count\n");
  for (i = 0; i < bins; i++)
    {
      printf ("[%+7.1f,%+7.1f), %6d:", range[i], range[i + 1], abs_bin_count[i]);
      for (j = 0;
           j < (abs_bin_count[i] / (double) max_abs_bin_count) * MAX_WIDTH;
           j++)
        {
          printf ("*");
        }
      printf ("\n");
 
    }
 
  /* Not all random numbers were in the range of the histogram */
  if (flag == 1)
    {
      printf
        ("\nWARNING: Random numbers generated outside range (%f, %f). These will not be shown in the above histogram.\n",
         min, max);
    }
 
 
  free (bin_count);
  free (range);
 
  printf ("mean= %g\n", sum/samples);
  printf ("positive mean= %g\n", abs_sum/samples);
  return 0;
}
