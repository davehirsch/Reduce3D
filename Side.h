// ===========================================================================
//	Side.h
// ===========================================================================
#pragma once

#import "NumberedPt.h"



// ---------------------------------------------------------------------------
//		¥ Side
// ---------------------------------------------------------------------------

class	Side {
	public:
						Side();
						Side(const Side &inSide);
		virtual			~Side();
		float			Area();
		float			MinAngle();
		Point3DFloat	& AwayVect(Point3DFloat &inPt);
		Point3DFloat	&Vect12to3();
		void			MakeInVect(Point3DFloat &inCtr);
		bool			PointOnSide(Point3DFloat &inPt);
		Side&			operator= (const Side &inSide);
		bool			operator== (const Side &inSide);

		

	public:
		NumberedPt		pt1, pt2, pt3;
		bool			edge1, edge2, edge3;
		Point3DFloat	inVect;
};
