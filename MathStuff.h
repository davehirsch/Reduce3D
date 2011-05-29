#pragma once

#import <math.h>

inline double_t	cos45() {return sqrt(2.0)/2.0;};

inline double_t sqr(double_t inA) {return pow(inA, 2);};

double_t RandomDbl(double_t inLower, double_t inUpper);

inline short RandomShort(short inMin, short inMax)
		{return trunc(RandomDbl(inMin,inMax+1));};

double_t CubeRoot(double_t x);

long factorial(long n);

/*
std::string &FltStr(double x, short decDigits);
std::string &FltFStr(double x, short decDigits);
std::string &TrimNL(std::string &inStr);
std::string &TrimWhite(std::string &inStr);
std::string &TrimWhite(std::string *inStr);
*/

#define dmh_min(A,B) ((A) < (B) ? (A) : (B))
#define dmh_max(A,B) ((A) > (B) ? (A) : (B))

#define ISNUMCHAR(A) (	(A == '+') ||						\
						(A == '-') ||						\
						(A == '.') ||						\
						(A == '0') ||						\
						((A >= '1') && (A <= '9')))
						

inline double myMin(double in1, double in2, double in3) {return dmh_min(in1, dmh_min(in2, in3));};
inline double myMax(double in1, double in2, double in3) {return dmh_max(in1, dmh_max(in2, in3));};

inline short sign(double in1) {return (in1 > 0 ? 1 : (in1 < 0 ? -1 : 0));};
