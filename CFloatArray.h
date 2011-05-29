// ===========================================================================
//	CFloatArray.h
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
