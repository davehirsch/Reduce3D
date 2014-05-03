// =================================================================================
//	NumPtArray.h 					
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// =================================================================================

#pragma once

#import <vector>



class NumberedPt;
class CrystalArray;

class NumPtArray {
	public:	// Exception Classes
		class ArrayIOErr {};

	public:
							NumPtArray();
							NumPtArray(CrystalArray *inXls);
							~NumPtArray();
		NumberedPt		&operator[](short inSub);
		bool			PointInArray(const NumberedPt &inPt);
		NumberedPt		PopPt();
		void			RemoveItem(int inAtIndex);
		void			PushPt(NumberedPt &inXl);
		void			Clear();
		inline int		GetCount() {return array.size();};
	void			Randomize();
	
	
	public:
		bool	Compare(const NumberedPt &inItem1, const NumberedPt &inItem2);
		static bool	AreEqual(const NumberedPt &inItem1, const NumberedPt &inItem2);
		
	private:
		std::vector<NumberedPt> array;
};



