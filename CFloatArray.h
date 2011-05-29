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
		float		Pop();
		void		Push(float *inNum);
		void		Push(float inNum);
		void		Clear();
		float		operator[](int inSub);
		void		Update(int index, float inNum);
		inline int		GetCount() {return array.size();};

private:
		std::vector<float> array;

};
