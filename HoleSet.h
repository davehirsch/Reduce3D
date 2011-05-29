// ===========================================================================
//	HoleSet.h
// ===========================================================================
#pragma once

#import "CrystalArray.h"
class stringFile;
class Calculator;

// ---------------------------------------------------------------------------
//		¥ HoleSet
// ---------------------------------------------------------------------------



class	HoleSet : public CrystalArray {
	public:
					HoleSet(Calculator *inCalc, long inNumExpected, stringFile *inFile);
					~HoleSet();
		bool		PointInHole(Point3DFloat &inPoint);
		double		NearestHoleDist(Point3DFloat &inPoint);
		void		TidyCrystalsUp(CrystalArray *inXls);
};
