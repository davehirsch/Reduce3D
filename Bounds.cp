// ===========================================================================//	Bounds.cp// ===========================================================================//	#import "Bounds.h"#import <float.h>// ---------------------------------------------------------------------------//		� Bounds// ---------------------------------------------------------------------------//	Default ConstructorBounds::Bounds(){	min = DBL_MAX;	max = DBL_MIN;	sum = 0.0;	sumsq = 0.0;}// ---------------------------------------------------------------------------//		� Bounds// ---------------------------------------------------------------------------//	Default DestructorBounds::~Bounds(){}// ---------------------------------------------------------------------------//		� Update// ---------------------------------------------------------------------------voidBounds::Update(double inVal){	if (inVal < min)		min = inVal;	if (inVal > max)		max = inVal;	sum += inVal;	sumsq += inVal * inVal;}