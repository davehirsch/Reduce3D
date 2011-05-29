// ===========================================================================
//	Crystal.h
// ===========================================================================
#pragma once

#import "Point3DFloat.h"



class Crystal {
	public:
						Crystal();
						Crystal(const Crystal &inXl);
		Crystal		operator= (const Crystal &inXl);
		bool		operator== (Crystal &inXl);
		inline bool	operator!= (Crystal &inXl) {return !(inXl == *this);};
		inline bool		PointInside (Point3DFloat &inPt) {return (ctr.Distance(inPt) < r);};

	public:		// I'm making these public, in violation of good style,
					// in order to keep function calls to a minimum
		Point3DFloat	ctr;		/* the coordinates of the center of the sphere */
		double r;			/* The sphere's radius */
		double actV;		// an adjusted volume to account for impingement.  If two crystals are impinged then their
						// extV will be too high, because each has part of its volume reduced by its neighbor.
		double extV;		// a simple volume calculation based on the radius
		double ctcDist;
		double polyV;
		double diffV;
		short neighbor;
		short ctrSlice;	/* The slice in which the center of the sphere lies */
		long ctrID;		/* The ID of the circle in which the center of the sphere lies */
};
