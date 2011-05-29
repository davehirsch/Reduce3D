// =================================================================================
//	NumberedPtComp.h 					
// =================================================================================

#pragma once

#import <vector>


// =================================================================================
//	NumPtArray.h 					
// =================================================================================

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
		bool	NumPtArray::Compare(const NumberedPt &inItem1, const NumberedPt &inItem2);
		static bool	NumPtArray::AreEqual(const NumberedPt &inItem1, const NumberedPt &inItem2);
		
	private:
		std::vector<NumberedPt> array;
};



