// ===========================================================================
//	Point3DFloat.h
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================
#pragma once
#import <math.h>
#import "Mathstuff.h"

class Point3DFloat {
	public:
		class		TrigErr{};

							Point3DFloat();
							Point3DFloat(double inX, double inY, double inZ);
							Point3DFloat(const Point3DFloat &inPt);
		inline void			Set(double inX, double inY, double inZ) {x = inX; y = inY; z = inZ;};
		inline void			Clear() {this->Set(0.0f,0.0f,0.0f);};

		inline double	Magnitude() {return sqrt(x * x + y * y + z * z);};
		inline Point3DFloat	&Unit() {return (*this)/(*this).Magnitude();};
		inline double	Distance(Point3DFloat &inPoint) {return (inPoint - (*this)).Magnitude();};
			// projection of this onto inPt vectorwise
		inline Point3DFloat	Proj(Point3DFloat &inPt) {return (inPt.Unit() * Comp(inPt));};
			//	component of this in the inPt direction
		inline double	Comp(Point3DFloat &inPt) {return (inPt.Unit() * *this);};
		double				SmallestOrthComp ();
		double				Angle (const Point3DFloat &inVect);

		// OPERATOR OVERLOADING
				// Assignment  (Note that this might be bad programming, but I'm not sure why
		Point3DFloat&	operator= (const Point3DFloat &inPt);

				// Subtraction
		Point3DFloat	&operator- (const Point3DFloat &inPt);
		inline void		operator-= (const Point3DFloat &inPt) {x -= inPt.x;y -= inPt.y;z -= inPt.z;};

		Point3DFloat	&operator- ();	// unary -

				// Addition
		Point3DFloat	&operator+ (const Point3DFloat &inPt);
		inline void		operator+= (const Point3DFloat &inPt) {x += inPt.x;y += inPt.y;z += inPt.z;};
		Point3DFloat	&operator+ (double inVal);
		inline void		operator+= (double inVal) {x += inVal;y += inVal;z += inVal;};

				// Dot Product (*)
		inline double	operator* (const Point3DFloat &inPt)
								{return (x * inPt.x + y * inPt.y + z * inPt.z);};

				// Cross Product (%)
		Point3DFloat	&operator% (const Point3DFloat &inPt);
		
				// Scalar Multiplication (force type to double)
		inline friend Point3DFloat
							&operator* (double inScale, Point3DFloat &inPt){return inPt * inScale;};
		Point3DFloat	&operator* (double inScale);
		inline void		operator*= (double inScale){x*=inScale;y*=inScale;z*=inScale;};

				// Scalar Division
		Point3DFloat	&operator/ (double inScale);
		inline void		operator/= (double inScale){x/=inScale;y/=inScale;z/=inScale;};
				// Comparison Operators
		inline bool	operator== (const Point3DFloat &inPt)
								{return inPt.x == x && inPt.y == y && inPt.z == z;};
		inline bool	operator!= (const Point3DFloat &inPt)
								{return (inPt.x != x || inPt.y != y || inPt.z != z);};
						
	public:		// I'm making these public, in violation of good style,
					// in order to keep function calls to a minimum
		double	x;
		double	y;
		double	z;
};