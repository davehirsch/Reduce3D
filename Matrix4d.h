// ===========================================================================
//	Matrix4d.h
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================
#pragma once

#import "Vector4d.h"
#import "NumberedPt.h"

// ---------------------------------------------------------------------------
//		¥ Matrix4d
// ---------------------------------------------------------------------------

class	Matrix4d {
	public:
		class OutOfBoundsErr{};	// exception class
						Matrix4d();
						Matrix4d(const Matrix4d &inMatrix4d);
		virtual			~Matrix4d();
		Vector4d		&operator[](short inSub);

		Point3DFloat	operator* (const Point3DFloat &inPt);
		NumberedPt		operator* (const NumberedPt &inPt);
		Matrix4d		operator* (const Matrix4d &inMat);
		void			BeIdentity();
		inline friend Point3DFloat
						operator* (Point3DFloat &inPt, Matrix4d &inMat) {return inMat*inPt;};
		inline friend NumberedPt
						operator* (NumberedPt &inPt, Matrix4d &inMat) {return inMat*inPt;};
		Matrix4d		RotationMatrix (Point3DFloat &pt1, Point3DFloat &pt2,
									 Point3DFloat &pt3);
		Matrix4d		TransformationMatrix (Point3DFloat &pt1, Point3DFloat &pt2,
									 Point3DFloat &pt3);
		Matrix4d		TranslationMatrix (double dx, double dy, double dz);
		Vector4d *		GetVector(short inSub);

	public:
		Vector4d		theArray[5];	// 5 vectors so we don't have to worry about starting
										// with zero instead of one; same thing is done in
										// Vector4d class. (ignore element zero)
};