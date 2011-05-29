// ===========================================================================
//	NumberedPt.h
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================
#pragma once

#import "Point3DFloat.h"

class NumberedPt : public Point3DFloat {
	public:
						NumberedPt();
						NumberedPt(short inSeq, double inX, double inY, double inZ, bool inFlag);
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
	bool flag;		// a flag to indicate whether it's been allocated to a hull facet
};