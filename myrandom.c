#include "myrandom.h"

#include <math.h>
#include <stdlib.h>

double
randn (double mu, double sigma)
/*
http://phoxis.org/2013/05/04/generating-random-numbers-from-normal-distribution-in-c/
*/
{
  double u1, u2, w, mult;
  static double x1, x2;
  static int call = 0;

  if (call == 1)
    {
      call = !call;
      return (mu + sigma * (double) x2);
    }

  do
    {
      u1 = -1 + ((double) rand () / RAND_MAX) * 2;
      u2 = -1 + ((double) rand () / RAND_MAX) * 2;
      w = pow (u1, 2) + pow (u2, 2);
    }
  while (w >= 1 || w == 0);

  mult = sqrt ((-2 * log (w)) / w);
  x1 = u1 * mult;
  x2 = u2 * mult;

  call = !call;

  return (mu + sigma * (double) x1);
}
 
double
randSkew( double mu, double scale, double skew )
/*
http://stackoverflow.com/questions/4643285/how-to-generate-random-numbers-that-follow-skew-normal-distribution-in-matlab
*/
{
  double sigma, u0, u1, v;
  sigma = skew / sqrt( 1 + skew*skew );
  u0 = randn(0,1);
  v = randn(0,1);
  u1 = sigma * u0 + sqrt(1-sigma*sigma)*v;
  return mu + ( u0 >= 0 ? u1*scale : -u1*scale );
}
