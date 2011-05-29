// ===========================================================================
//	HoleSet.cp
// ===========================================================================
//	
#include "HoleSet.h"
#import "Calculator.h"
#import "stringFile.h"

// ---------------------------------------------------------------------------
//		¥ HoleSet
// ---------------------------------------------------------------------------
//	Default Constructor

HoleSet::HoleSet(Calculator *inCalc, long inNumExpected, stringFile *inFile)
	: CrystalArray(inCalc, inNumExpected)
{
	SetFile(inFile);
	ReadMergeFile();
}

// ---------------------------------------------------------------------------
//		¥ HoleSet
// ---------------------------------------------------------------------------
//	Default Destructor

HoleSet::~HoleSet()
{
}

// ---------------------------------------------------------------------------
//		¥ PointInHole
// ---------------------------------------------------------------------------
bool
HoleSet::PointInHole(Point3DFloat &inPoint)
{
	Crystal *curXl;
	for (int i = 0; i <= GetNumXls() - 1; i++) {
		curXl = (Crystal *) GetItemPtr(i);
		if ((curXl->ctr.Distance(inPoint) < curXl->r))
			return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
//		¥ NearestHoleDist
// ---------------------------------------------------------------------------
// returns distance from inPoint to surface of nearest hole (not center)
double
HoleSet::NearestHoleDist(Point3DFloat &inPoint)
{
	double minDist = HUGE_VAL;
	double testDist;
	short numHoles = GetNumXls();
	Crystal *curXl;

	for (int i = 0; i <= numHoles; i++) {
		curXl = (Crystal *) GetItemPtr(i);
		testDist = curXl->ctr.Distance(inPoint) - curXl->r;
		if (minDist > testDist)
			minDist = testDist;
	}
	return minDist;
}

// ---------------------------------------------------------------------------
//		¥ TidyCrystalsUp
// ---------------------------------------------------------------------------
// removes any crystals whose centers are inside a hole
void
HoleSet::TidyCrystalsUp(CrystalArray *inXls)
{
	mCalc->setupProgress("Removing crystals inside holes", nil, nil, nil, -1, 0, GetNumXls(), 0, false);	 
	Crystal *thisHole, *otherXl;
	int numHoles = GetNumXls();
	for (int i = 0; i <= numHoles; i++) {
		thisHole = (Crystal *) GetItemPtr(i);
		mCalc->progress(i);
		for (int j = 0; j <= inXls->GetNumXls()-1; j++) {
			otherXl = (Crystal *) inXls->GetItemPtr(j);
			float separation = thisHole->ctr.Distance(otherXl->ctr);
			if (separation < thisHole->r)	{	// if the crystal center is inside the hole
				inXls->RemoveItem(j, false);
				j--;	// redo this j, since we've removed an element
			}
		}
	}
	inXls->RebuildList();
}