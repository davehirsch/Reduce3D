// ===========================================================================
//	ShuffleArray.cp
//		An array of indices 0 to numElements-1, in a random order using the C++ STL
//		for storage.
// ===========================================================================
//	
#import	"ShuffleArray.h"
#import "MathStuff.h"
#import <algorithm>


// ---------------------------------------------------------------------------
//		¥ ShuffleArray
// ---------------------------------------------------------------------------
//	Constructor for a ShuffleArray

ShuffleArray::ShuffleArray(short numElements)
{
	array.reserve(numElements);

	for (int i = 0; i <= numElements - 1; i++)
		Push(i);
	
	std::random_shuffle ( array.begin(), array.end() );
}

// ---------------------------------------------------------------------------
//		¥ ~ShuffleArray
// ---------------------------------------------------------------------------
//	Destructor for ShuffleArray

ShuffleArray::~ShuffleArray()
{
}

// ---------------------------------------------------------------------------------
//		¥ Pop
// ---------------------------------------------------------------------------------
short
ShuffleArray::Pop()
{
	static short outNum;
	
	if (array.size() > 0) {
		outNum = array.back();
		array.pop_back();
	} else {		
		throw ArrayIOErr();
	}
	
	return outNum;
}

// ---------------------------------------------------------------------------------
//		¥ Push
// ---------------------------------------------------------------------------------
void
ShuffleArray::Push(short *inNum)
{
	array.push_back(*inNum);
}

// ---------------------------------------------------------------------------------
//		¥ Push
// ---------------------------------------------------------------------------------
void
ShuffleArray::Push(short inNum)
{
	array.push_back(inNum);
}

// ---------------------------------------------------------------------------------
//		¥ Clear
// ---------------------------------------------------------------------------------
void
ShuffleArray::Clear()
{
	array.clear();
}

// ---------------------------------------------------------------------------
//		¥ operator[]
// ---------------------------------------------------------------------------
short
ShuffleArray::operator[](short inSub)
{
	return array.at(inSub);
}

// ---------------------------------------------------------------------------
//		¥ Update
// ---------------------------------------------------------------------------
void
ShuffleArray::Update(short index, short inNum)
{
	if (index > 0 && index < array.size()){
		array.at(index) = inNum;
	}
}

