// ===========================================================================
//  MathStuff.cp
//  Reduce3D
//
//  Created by David Hirsch on 11/25/07.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================

#import "MathStuff.h"
#import <stdlib.h>

long factorial(long n)
{
	if ((n == 1) || (n == 0))
		return 1;
	else
		return (n * factorial(n-1));
}

double_t CubeRoot(double_t x)
{
	return ((x < 0) ?	// if x < 0
		(-1.0 * pow((double_t)-x, (double_t)(1.0/3.0)))	// return -((-x)^(1/3))
		: ( (x==0) ?	// else if x == 0
			0	// return zero, regardless of y
			: pow((double_t)x, (double_t)(1.0/3.0))));	// else (x > 0), so return x^(1/3)
}


double_t RandomDbl(double_t lower, double_t upper)
{
	double_t test=((drand48() * (upper - lower)) + lower);
	return test;
}