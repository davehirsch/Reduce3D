// =================================================================================
//	NumPtArray.cp 						
//		A 1-based array of NumberedPt's using the C++ STL for storage.  Used for legacy
//		(CW PowerPlant) compatibility
// =================================================================================

#import "NumPtArray.h"
#import "NumberedPt.h"
#import <algorithm>
#import "CrystalArray.h"
#import "ShuffleArray.h"

// ---------------------------------------------------------------------------
//		¥ NumPtArray
// ---------------------------------------------------------------------------
//	Constructor for an empty NumPtArray

NumPtArray::NumPtArray()
{
	// reserve space for the array
	array.reserve(1000);
}

// ---------------------------------------------------------------------------
//		¥ NumPtArray
// ---------------------------------------------------------------------------
//	Copy Constructor (CrystalArray) for an empty NumPtArray

NumPtArray::NumPtArray(CrystalArray *inXls)
{
	Crystal * thisXl;
	NumberedPt	thisPt;
	
	int numXls = inXls->GetNumXls();
	array.reserve(numXls);
	
	for (short i=0;i <= numXls-1; i ++) {
		thisXl = inXls->GetItemPtr(i);
		thisPt.x = thisXl->ctr.x;
		thisPt.y = thisXl->ctr.y;
		thisPt.z = thisXl->ctr.z;
		thisPt.seq = i;
		PushPt(thisPt);
	}
}


// ---------------------------------------------------------------------------
//		¥ ~NumPtArray
// ---------------------------------------------------------------------------
//	Destructor for NumPtArray

NumPtArray::~NumPtArray()
{
	// array should free memory automatically
}

// ---------------------------------------------------------------------------------
//		¥ PointInArray
// ---------------------------------------------------------------------------------
bool
NumPtArray::PointInArray(const NumberedPt &inPt)
{
	std::vector<NumberedPt>::iterator it;
	it = std::find(array.begin(), array.end(), inPt);
	return (it != array.end());
}

// ---------------------------------------------------------------------------------
//		¥ PushPt
// ---------------------------------------------------------------------------------
void
NumPtArray::PushPt(NumberedPt &inXl)
{
	array.push_back(inXl);
}

// ---------------------------------------------------------------------------------
//		¥ PopPt
// ---------------------------------------------------------------------------------
NumberedPt
NumPtArray::PopPt()
{
	static NumberedPt outXl;
	
	if (array.size() > 0) {
		outXl = array.back();
		array.pop_back();
	} else {		
		throw ArrayIOErr();
	}

	return outXl;

}

// ---------------------------------------------------------------------------------
//		¥ RemoveItem
// ---------------------------------------------------------------------------------
void
NumPtArray::RemoveItem(int inAtIndex)
{	
	array.erase(array.begin() + inAtIndex);
}

// ---------------------------------------------------------------------------------
//		¥ Clear
// ---------------------------------------------------------------------------------
void
NumPtArray::Clear()
{
	array.clear();
}

// ---------------------------------------------------------------------------
//		¥ operator[]
// ---------------------------------------------------------------------------
NumberedPt &
NumPtArray::operator[](short inSub)
{
	static NumberedPt outPt;
	outPt = array.at(inSub);
	return outPt;
}

// ---------------------------------------------------------------------------------
//		¥ Compare
//	Comparator based on sequence numbers, not locations
// ---------------------------------------------------------------------------------
bool
NumPtArray::Compare(const NumberedPt &inItem1, const NumberedPt &inItem2)
{
	return (inItem1.seq < inItem2.seq);
}

// ---------------------------------------------------------------------------------
//		¥ AreEqual
//	Comparator based on sequence numbers, not locations
// ---------------------------------------------------------------------------------
bool
NumPtArray::AreEqual(const NumberedPt &inItem1, const NumberedPt &inItem2)
{
	return (inItem1.seq == inItem2.seq);
}

// ---------------------------------------------------------------------------------
//		¥ Randomize
// ---------------------------------------------------------------------------------
void
NumPtArray::Randomize()
{
	ShuffleArray shuffler( GetCount() );
	for (short i=0; i<GetCount(); i++) {
		NumberedPt temp = array.at(shuffler[i]);
		array.at(shuffler[i]) = array.at(i);
		array.at(i) = temp;
	}
}
