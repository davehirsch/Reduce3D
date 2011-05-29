// ===========================================================================
//	ShuffleArray.h
// ===========================================================================
#pragma once

#import <vector>

// ---------------------------------------------------------------------------
//		¥ ShuffleArray
// ---------------------------------------------------------------------------

class	ShuffleArray  {
	public:
		class		ArrayIOErr{};

					ShuffleArray(short numElements);
					~ShuffleArray();
		short		Pop();
		void		Push(short inNum);
		void		Push(short *inNum);
		void		Clear();
		void		Shuffle();
		short		operator[](short inSub);
		void		Update(short index, short inNum);

	private:
		std::vector<short> array;
};
