// ===========================================================================
//	Point3DFloat.cp
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================
#import "Point3DFloat.h"

// ---------------------------------------------------------------------------
//		� Point3DFloat
// ---------------------------------------------------------------------------
Point3DFloat::Point3DFloat()
{
	x = y = z = 0.0;
}

// ---------------------------------------------------------------------------
//		� Point3DFloat
// ---------------------------------------------------------------------------
Point3DFloat::Point3DFloat(double inX, double inY, double inZ)
{
	x = inX;
	y = inY;
	z = inZ;
}

// ---------------------------------------------------------------------------
//		� Point3DFloat
// ---------------------------------------------------------------------------
Point3DFloat::Point3DFloat(const Point3DFloat &inPt)
{
	x = inPt.x;
	y = inPt.y;
	z = inPt.z;
}

// ---------------------------------------------------------------------------
//		� Clear  --> inlined
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//		� operator -
// ---------------------------------------------------------------------------
// does a vector subtraction
Point3DFloat &
Point3DFloat::operator- (const Point3DFloat &inPt)
{
	static Point3DFloat diff;
	diff.x = x - inPt.x;
	diff.y = y - inPt.y;
	diff.z = z - inPt.z;
	return diff;
}

// ---------------------------------------------------------------------------
//		� operator -
// ---------------------------------------------------------------------------
// does a vector negation
Point3DFloat &
Point3DFloat::operator- ()
{
	static Point3DFloat diff;
	diff.x = -x;
	diff.y = -y;
	diff.z = -z;
	return diff;
}

// ---------------------------------------------------------------------------
//		� operator +
// ---------------------------------------------------------------------------
// does a vector addition
Point3DFloat &
Point3DFloat::operator+ (const Point3DFloat &inPt)
{
	static Point3DFloat sum;
	sum.x = x + inPt.x;
	sum.y = y + inPt.y;
	sum.z = z + inPt.z;
	return sum;
}

// ---------------------------------------------------------------------------
//		� operator +
// ---------------------------------------------------------------------------
// adds a scalar to each dimension
Point3DFloat &
Point3DFloat::operator+ (double inVal)
{
	static Point3DFloat sum;
	sum.x = x + inVal;
	sum.y = y + inVal;
	sum.z = z + inVal;
	return sum;
}

// ---------------------------------------------------------------------------
//		� operator %
// ---------------------------------------------------------------------------
// does a vector multiplication ->> Cross Product!
Point3DFloat &
Point3DFloat::operator% (const Point3DFloat &inPt)
{
	static Point3DFloat prod;
	prod.x = y * inPt.z - inPt.y * z;
	prod.y = z * inPt.x - inPt.z * x;
	prod.z = x * inPt.y - inPt.x * y;
	return prod;
}

// ---------------------------------------------------------------------------
//		� operator * (double)
// ---------------------------------------------------------------------------
// does a scalar multiplication
Point3DFloat &
Point3DFloat::operator* (double inScale)
{
	static Point3DFloat prod;
	prod.x = x * inScale;
	prod.y = y * inScale;
	prod.z = z * inScale;
	return prod;
}

// ---------------------------------------------------------------------------
//		� operator / (double)
// ---------------------------------------------------------------------------
// does a scalar division
Point3DFloat &
Point3DFloat::operator/ (double inScale)
{
	static Point3DFloat quot;
	quot.x = x / inScale;
	quot.y = y / inScale;
	quot.z = z / inScale;
	return quot;
}

// ---------------------------------------------------------------------------
//		� operator== (const Point3DFloat &) ->>inlined!
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//		� operator!= (const Point3DFloat &) ->>inlined!
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//		� operator= (const Point3DFloat &)
// ---------------------------------------------------------------------------
Point3DFloat &
Point3DFloat::operator= (const Point3DFloat &inPt)
{
	static Point3DFloat outPt;
	(*this).x = inPt.x;
	(*this).y = inPt.y;
	(*this).z = inPt.z;
	outPt.x = inPt.x;
	outPt.y = inPt.y;
	outPt.z = inPt.z;
	return outPt;
}

// ---------------------------------------------------------------------------
//		� Magnitude ->>inlined!
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//		� Unit ->>inlined!
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//		� Distance ->>inlined!
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//		� operator * (double) ->>inlined!
// ---------------------------------------------------------------------------
// does a scalar multiplication

// ---------------------------------------------------------------------------
//		� operator += ->> inlined!
// ---------------------------------------------------------------------------
// does a vector addition to self

// ---------------------------------------------------------------------------
//		� operator -= ->> inlined!
// ---------------------------------------------------------------------------
// does a vector subtraction from self

// ---------------------------------------------------------------------------
//		� operator * ->> inlined!
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//		� SmallestOrthComp ()
// ---------------------------------------------------------------------------
double
Point3DFloat::SmallestOrthComp ()
{
	Point3DFloat unitX(1, 0, 0);
	Point3DFloat unitY(0, 1, 0);
	Point3DFloat unitZ(0, 0, 1);
	return myMin(this->Comp(unitX), this->Comp(unitY), this->Comp(unitZ));
}

// ---------------------------------------------------------------------------
//		� Angle (const Point3DFloat &)
// ---------------------------------------------------------------------------
/* treating self and inVect as vectors, returns the angle between them (0 to pi) */
double
Point3DFloat::Angle (const Point3DFloat &inVect)
{
	double dot = (*this) * inVect;
	dot /= this->Magnitude();
	Point3DFloat tempPt(inVect);
	dot /= tempPt.Magnitude();
	if (dot < -1.1 || dot > 1.1)
		throw TrigErr();
	if (dot <= -1.0)
		return M_PI;
	if (dot >= 1.0)
		return 0;
	
	return acos(dot);
}
