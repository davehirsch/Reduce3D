// ===========================================================================
//	Side.h
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================
#pragma once

#import "NumberedPt.h"



// ---------------------------------------------------------------------------
//		� Side
// ---------------------------------------------------------------------------

class	Side {
	public:
						Side();
						Side(const Side &inSide);
		virtual			~Side();
		double			Area();
		double			MinAngle();
		Point3DFloat	& AwayVect(Point3DFloat &inPt);
		Point3DFloat	&Vect12to3();
		void			MakeInVect(Point3DFloat &inCtr);
		bool			PointOnSide(Point3DFloat &inPt);
		Side&			operator= (const Side &inSide);
		bool			operator== (const Side &inSide);
		bool			IsWithout(Point3DFloat &inPt);
		Point3DFloat &	CenterOfMass();

		

	public:
		NumberedPt		pt1, pt2, pt3;
		bool			edge1, edge2, edge3;
		Point3DFloat	inVect;
};
