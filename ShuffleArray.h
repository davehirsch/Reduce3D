// ===========================================================================
//	ShuffleArray.h
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
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
