// ===========================================================================
//	NumberedPt.h
// ===========================================================================
#pragma once

#import "Point3DFloat.h"

class NumberedPt : public Point3DFloat {
	public:
						NumberedPt();
						NumberedPt(short inSeq, float inX, float inY, float inZ);
						NumberedPt(const NumberedPt &inPt);
						NumberedPt(const Point3DFloat &inPt);
		NumberedPt	&operator= (NumberedPt &inPt);
		NumberedPt	&operator= (Point3DFloat &inPt);
		void		Copy (NumberedPt &inPt);
		bool		operator== (const NumberedPt &inPt);
		bool		operator!= (NumberedPt &inPt);
			
	public:		// I'm making these public, in violation of good style,
					// in order to keep function calls to a minimum
		short seq;
};