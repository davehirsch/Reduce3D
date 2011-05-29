// ===========================================================================
//	CFloatArray.h
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================
#pragma once
#import <vector>

//#import "fp.h"
// ---------------------------------------------------------------------------
//		¥ CFloatArray
// ---------------------------------------------------------------------------



class	CFloatArray {
	public:
		class		ArrayIOErr{};

					CFloatArray();
					~CFloatArray();
		double		Pop();
		void		Push(double *inNum);
		void		Push(double inNum);
		void		Clear();
		double		operator[](int inSub);
		void		Update(int index, double inNum);
		inline int		GetCount() {return array.size();};

private:
		std::vector<double> array;

};
