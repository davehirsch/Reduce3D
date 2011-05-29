// ===========================================================================
//	HoleSet.h
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
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
