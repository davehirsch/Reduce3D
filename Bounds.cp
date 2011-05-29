// ===========================================================================//	Bounds.cp//  Reduce3D////  Created by David Hirsch on 10/1/97.//  Copyright 2011 David Hirsch.//  Distributed under the terms of the GNU General Public License v3//	See file "COPYING for more info.// ===========================================================================#import "Bounds.h"#import <float.h>// ---------------------------------------------------------------------------//		� Bounds// ---------------------------------------------------------------------------//	Default ConstructorBounds::Bounds(){	min = DBL_MAX;	max = DBL_MIN;	sum = 0.0;	sumsq = 0.0;}// ---------------------------------------------------------------------------//		� Bounds// ---------------------------------------------------------------------------//	Default DestructorBounds::~Bounds(){}// ---------------------------------------------------------------------------//		� Update// ---------------------------------------------------------------------------voidBounds::Update(double inVal){	if (inVal < min)		min = inVal;	if (inVal > max)		max = inVal;	sum += inVal;	sumsq += inVal * inVal;}