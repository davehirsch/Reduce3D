// ===========================================================================
//	SideSet.cp
// ===========================================================================
//
#import	"SideSet.h"
#import	"Matrix4d.h"
#import "CrystalArray.h"
#import "CoreFoundation/CoreFoundation.h"

// ---------------------------------------------------------------------------
//		¥ SideSet
// ---------------------------------------------------------------------------
//	Constructor for an empty SideSet

SideSet::SideSet()
{
	mType = kSidesBox;
	mSideLen = 0.0;
	mSideLenX = 0.0;
	mSideLenY = 0.0;
	mSideLenZ = 0.0;
	mHeight = 0.0;
	mRadius = 0.0;
	mInscribedBox = nil;
	mExscribedBox = nil;
	// reserve space for the array
	array.reserve(600);
}

// ---------------------------------------------------------------------------
//		¥ SideSet
// ---------------------------------------------------------------------------
//	Copy Constructor

SideSet::SideSet(SideSet &inSideSet)
{
	mType = inSideSet.mType;
	mCtr = inSideSet.mCtr;
	mSideLen = inSideSet.mSideLen;
	mSideLenX = inSideSet.mSideLenX;
	mSideLenY = inSideSet.mSideLenY;
	mSideLenZ = inSideSet.mSideLenZ;
	mHeight = inSideSet.mHeight;
	mRadius = inSideSet.mRadius;
	mOffset = inSideSet.mOffset;
	array = inSideSet.array;
//	mInscribedBox = (inSideSet.mInscribedBox == nil ? nil : new SideSet(*(inSideSet.mInscribedBox)));
//	mExscribedBox = (inSideSet.mExscribedBox == nil ? nil : new SideSet(*(inSideSet.mExscribedBox)));
	if (inSideSet.mInscribedBox != nil) {
		mInscribedBox = new SideSet();
		mInscribedBox->Copy(*(inSideSet.mInscribedBox));
	}
	if (inSideSet.mExscribedBox) {
		mExscribedBox = new SideSet();
		mExscribedBox->Copy(*(inSideSet.mExscribedBox));
	}
}

// ---------------------------------------------------------------------------
//		¥ Copy
// ---------------------------------------------------------------------------
void
SideSet::Copy(SideSet &inSideSet)
{
	mType = inSideSet.mType;
	mCtr = inSideSet.mCtr;
	mSideLen = inSideSet.mSideLen;
	mSideLenX = inSideSet.mSideLenX;
	mSideLenY = inSideSet.mSideLenY;
	mSideLenZ = inSideSet.mSideLenZ;
	mHeight = inSideSet.mHeight;
	mRadius = inSideSet.mRadius;
	mOffset = inSideSet.mOffset;
	array = inSideSet.array;
	if (mInscribedBox != nil)
		delete mInscribedBox;
	if (inSideSet.mInscribedBox != nil) {
		mInscribedBox = new SideSet();
		mInscribedBox->Copy(*(inSideSet.mInscribedBox));
	}
	if (mExscribedBox != nil)
		delete mExscribedBox;
	if (inSideSet.mExscribedBox) {
		mExscribedBox = new SideSet();
		mExscribedBox->Copy(*(inSideSet.mExscribedBox));
	}
}

// ---------------------------------------------------------------------------
//		¥ ~SideSet
// ---------------------------------------------------------------------------
//	Destructor for SideSet

SideSet::~SideSet()
{
	if (mInscribedBox != nil) {
		delete mInscribedBox;
		mInscribedBox = nil;
	}
	if (mExscribedBox != nil) {
		delete mExscribedBox;
		mExscribedBox = nil;
	}

	// (array should free memory automatically)
}


// ---------------------------------------------------------------------------------
//		¥ PopSide
// ---------------------------------------------------------------------------------
Side
SideSet::PopSide()
{
	static Side outSide;
	
	if (array.size() > 0) {
		outSide = array.back();
		array.pop_back();
	} else {		
		throw ArrayIOErr();
	}
	
	return outSide;
}

// ---------------------------------------------------------------------------------
//		¥ RemoveSide
// ---------------------------------------------------------------------------------
void
SideSet::RemoveSide(int inAtIndex)
{	
	array.erase(array.begin() + inAtIndex);
}

// ---------------------------------------------------------------------------------
//		¥ PushSide
// ---------------------------------------------------------------------------------
void
SideSet::PushSide(Side &inSide)
{
	array.push_back(inSide);
}

// ---------------------------------------------------------------------------------
//		¥ Clear
// ---------------------------------------------------------------------------------
void
SideSet::Clear()
{
	array.clear();
}

// ---------------------------------------------------------------------------
//		¥ operator[]
// ---------------------------------------------------------------------------
Side &
SideSet::operator[](short inSub)
{
	return array.at(inSub);
}

// ---------------------------------------------------------------------------
//		¥ GetItemPtr
// ---------------------------------------------------------------------------
Side *			
SideSet::GetItemPtr(short inSub)
{
	return &(array.at(inSub));
}

// ---------------------------------------------------------------------------
//		¥ Volume
// ---------------------------------------------------------------------------
// Returns the volume of the bounding box.  Note that if there are overlapping
//	triangles on planes, the value will be too high
float
SideSet::Volume()
{
	float	vol = 0.0;
	Side	curSide;
	int		numSides = array.size();
	switch (mType) {
		case kSidesBox:
			for (int i = 0; i <= numSides - 1; i++) {
				curSide = (*this)[i];
				vol += TetrahedronVolume(curSide.pt1, curSide.pt2, curSide.pt3, mCtr);
			}
			return vol;
		break;
		case kCubeBox:
			return pow(mSideLen, 3);
		break;
		case kRPBox:
			return (mSideLenX * mSideLenY * mSideLenZ);
		break;
		case kCylBox:
			return (mHeight * M_PI * mRadius * mRadius);
		break;
	}
	return -1.0;
}


// ---------------------------------------------------------------------------
//		¥ TetrahedronVolume
// ---------------------------------------------------------------------------
// Returns the volume of the tetrahedron formed by points pt1,pt2,pt3,pt4.
// Formula obtained from CRC math tables, 25th edition, p.295
float
SideSet::TetrahedronVolume (Point3DFloat &pt1, Point3DFloat &pt2,
								Point3DFloat &pt3, Point3DFloat &pt4)
{
	float vol;

		vol =  pt1.x * (pt2.y * pt3.z + pt2.z * pt4.y + pt3.y * pt4.z - pt3.z
							* pt4.y - pt2.z * pt3.y - pt2.y * pt4.z);
		vol -= pt2.x * (pt1.y * pt3.z + pt1.z * pt4.y + pt3.y * pt4.z - pt3.z
							* pt4.y - pt1.z * pt3.y - pt1.y * pt4.z);
		vol += pt3.x * (pt1.y * pt2.z + pt1.z * pt4.y + pt2.y * pt4.z - pt2.z
							* pt4.y - pt1.z * pt2.y - pt1.y * pt4.z);
		vol -= pt4.x * (pt1.y * pt2.z + pt1.z * pt3.y + pt2.y * pt3.z - pt2.z
							* pt3.y - pt1.z * pt2.y - pt1.y * pt3.z);
	return ::fabs(vol / 6.0);
}

// ---------------------------------------------------------------------------
//		¥ SetDimensions
// ---------------------------------------------------------------------------
void
SideSet::SetDimensions(float inXLen, float inYLen, float inZLen)
{
	mSideLenX = inXLen;
	mSideLenY = inYLen;
	mSideLenZ = inZLen;
}

// ---------------------------------------------------------------------------
//		¥ SetDimensions
// ---------------------------------------------------------------------------
void
SideSet::SetDimensions(Point3DFloat inPt)
{
	mSideLenX = inPt.x;
	mSideLenY = inPt.y;
	mSideLenZ = inPt.z;
}
// ---------------------------------------------------------------------------
//		¥ SetDimensions
// ---------------------------------------------------------------------------
void
SideSet::SetDimensions(float inRadius, float inHeight)
{
	mRadius = inRadius;
	mHeight = inHeight;
}

// ---------------------------------------------------------------------------
//		¥ UpdateSide
// ---------------------------------------------------------------------------
void
SideSet::UpdateSide(short index, Side &inSide)
{
	array.at(index) = inSide;
}

// ---------------------------------------------------------------------------
//		¥ AlignSide
// ---------------------------------------------------------------------------
// This procedure aligns a triangle so that it's cross product points towards
//	the center of the solid, instead  of away from it.  This amounts to putting
//	the three points in the correct order.
void
SideSet::AlignSide(Side &ioSide)
{
	NumberedPt	vect1, vect2, xP;
	NumberedPt tempPt;
	Point3DFloat	tempCtr = mCtr;

	tempCtr = mCtr - ioSide.pt1;
	vect1 = ioSide.pt2 - ioSide.pt1;
	vect2 = ioSide.pt3 - ioSide.pt1;
	xP = vect1 % vect2;
	if ((xP * tempCtr) < 0)	{
			tempPt = ioSide.pt2;
			ioSide.pt2 = ioSide.pt3;
			ioSide.pt3 = tempPt;
	}
}

// ---------------------------------------------------------------------------
//		¥ PointOnBox
// ---------------------------------------------------------------------------
/* Returns true if the inPt is one of those points that make up a side of a
	kSidesBox BBox */
bool
SideSet::PointOnBox(Point3DFloat &inPt)
{
	Side *thisSide;
	int		numSides = array.size();

	if (mType != kSidesBox)
		return false;
	else {
		for (int i = 0 ; i < numSides ; i ++) {
			thisSide = &(array.at(i));
			if (inPt == thisSide->pt1 || inPt == thisSide->pt2 || inPt == thisSide->pt3)
				return true;
		}
	}
	return false;
}

// ---------------------------------------------------------------------------
//		¥ MakeAllInVects
// ---------------------------------------------------------------------------
/* Creates a vector for each side of a SidesBox that is normal to the side and
	points towards the center. */
void
SideSet::MakeAllInVects()
{
	Side *thisSide;
	int		numSides = array.size();

	if (mType != kSidesBox)
		throw(0);
	else {
		for (int i = 0 ; i <= numSides - 1 ; i ++) {
			thisSide = &(array.at(i));
			thisSide->MakeInVect(mCtr);
		}
	}
}

// ---------------------------------------------------------------------------
//		¥ RawPointInBox
// ---------------------------------------------------------------------------
//
bool
SideSet::RawPointInBox(Point3DFloat &inPt, float tolerance)
{
	Side *thisSide;
	Point3DFloat	vect1, vect2, xP, tempVect, testpt;

	switch (mType) {
		case kCubeBox:
			return ((::fabs(mCtr.x + mOffset.x - inPt.x) <= (mSideLen / 2.0)) &&
					(::fabs(mCtr.y + mOffset.y - inPt.y) <= (mSideLen / 2.0)) &&
					(::fabs(mCtr.z + mOffset.z - inPt.z) <= (mSideLen / 2.0)));
		break;
		case kRPBox:
			return ((::fabs(mCtr.x + mOffset.x - inPt.x) <= (mSideLenX / 2.0)) &&
					(::fabs(mCtr.y + mOffset.y - inPt.y) <= (mSideLenY / 2.0)) &&
					(::fabs(mCtr.z + mOffset.z - inPt.z) <= (mSideLenZ / 2.0)));
		break;
		case kCylBox:
			return ((sqrt(sqr(mCtr.x + mOffset.x - inPt.x) +
						  sqr(mCtr.y + mOffset.y - inPt.y)) < mRadius) &&
					(::fabs(mCtr.z + mOffset.z - inPt.z) <= (mHeight / 2.0)));
		break;
		case kSidesBox:
			int		numSides = array.size();
			int i;
			for (i = 0 ; i <= numSides-1 ; i ++) {
				thisSide = &(array.at(i));
				tempVect = inPt - (thisSide->pt1 + mOffset);
				// Dot Product is positive if both vectors point within 90 degrees of each other...
//				if ((thisSide->inVect * tempVect) < 0 ) {
				if (thisSide->inVect.Angle(tempVect) > (M_PI_2 - tolerance) ) {	// positive tolerance values make more points register as being outisde the box
						return false;	// the point is outside this side
				}
			}
			return true;	// the point is inside all sides of the box
		break;
		default:
			throw(0);
		break;
	}
	throw("Shouldn't Be Here!");
	return true; 	// should never get here!
}

// ---------------------------------------------------------------------------
//		¥ AspectRatio
// ---------------------------------------------------------------------------
//
float
SideSet::AspectRatio()
{
	switch (mType) {
		case kCubeBox:
			return 1.0;
		break;
		case kRPBox:
			return myMax(mSideLenX, mSideLenY, mSideLenZ)
				/ myMin(mSideLenX, mSideLenY, mSideLenZ);
		break;
		case kCylBox:
			return dmh_max(mRadius, mHeight) / dmh_min(mRadius, mHeight);
		break;
		case kSidesBox:
			return -1;
		break;
	}
	throw("Shouldn't Be Here!");
	return true; 	// should never get here!
}

// ---------------------------------------------------------------------------
//		¥ AllPointsInBox
// ---------------------------------------------------------------------------
// Determines whether all the points in the CrystalArray are inside this box
bool
SideSet::AllPointsInBox(CrystalArray *theXls)
{
	short numXls = theXls->GetNumXls();
	for (int i=0; i <= numXls - 1; i++) {
		Crystal *thisXl = theXls->GetItemPtr(i);
		if (!RawPointInBox(thisXl->ctr)) {
			return false;
		}
	}
	return true;
}


// ---------------------------------------------------------------------------
//		¥ NumPointsInBox
// ---------------------------------------------------------------------------
//
short
SideSet::NumPointsInBox(CrystalArray *theXls)
{
	short numPts = 0;
	short numXls = theXls->GetNumXls();
	
	for (int i=0; i <= numXls - 1; i++) {
		Crystal *thisXl = theXls->GetItemPtr(i);
		if (RawPointInBox(thisXl->ctr)) {
			numPts++;
		}
	}
	return numPts;
}


// ---------------------------------------------------------------------------
//		¥ ToroidalDistance
// ---------------------------------------------------------------------------
float
SideSet::ToroidalDistance(Point3DFloat &inPt1, Point3DFloat &inPt2)
{
	float xDist, yDist, zDist;

	switch (mType) {
		case kCubeBox:
			xDist = ::fabs(inPt1.x - inPt2.x);
			xDist = (xDist <= mSideLen/2) ? xDist : (mSideLen - xDist);
			yDist = ::fabs(inPt1.y - inPt2.y);
			yDist = (yDist <= mSideLen/2) ? yDist : (mSideLen - yDist);
			zDist = ::fabs(inPt1.z - inPt2.z);
			zDist = (zDist <= mSideLen/2) ? zDist : (mSideLen - zDist);
			return sqrt(sqr(xDist) + sqr(yDist) + sqr(zDist));
		break;
		case kRPBox:
			xDist = ::fabs(inPt1.x - inPt2.x);
			xDist = (xDist <= mSideLenX/2) ? xDist : (mSideLenX - xDist);
			yDist = ::fabs(inPt1.y - inPt2.y);
			yDist = (yDist <= mSideLenY/2) ? yDist : (mSideLenY - yDist);
			zDist = ::fabs(inPt1.z - inPt2.z);
			zDist = (zDist <= mSideLenZ/2) ? zDist : (mSideLenZ - zDist);
			return sqrt(sqr(xDist) + sqr(yDist) + sqr(zDist));
		break;
		case kCylBox:
			return inPt1.Distance(inPt2);
		break;
		case kSidesBox:
			return inPt1.Distance(inPt2);
		break;
		default:
			throw;
		break;
	}
	throw("Shouldn't Be Here!");
	return -1;
}

// ---------------------------------------------------------------------------
//		¥ CalcCtr
// ---------------------------------------------------------------------------
/* This doesn't calculate a true center, just a point that is guaranteed to be inside the SideSet */
Point3DFloat &
SideSet::CalcCtr()
{
	static Point3DFloat outPt;
	
	short numSides = array.size();
	for (short i=0; i < numSides; i++) {
		Side *thisSide = GetItemPtr(i);
		outPt += thisSide->CenterOfMass();
	}
	outPt /= (float)numSides;
	return outPt;
}

// ---------------------------------------------------------------------------
//		¥ GetSideLenPt
// ---------------------------------------------------------------------------
Point3DFloat &
SideSet::GetSideLenPt()
{
	static Point3DFloat outPt;
	
	if (mType == kRPBox) {
		outPt.Set(mSideLenX, mSideLenY, mSideLenZ);
	} else {
		throw;
	}
	return outPt;
}
/*
// ---------------------------------------------------------------------------
//		¥ RandPtInPrimitive
// ---------------------------------------------------------------------------
void
SideSet::RandPtInPrimitive(Point3DFloat &ioPt)
{
	switch (mType) {
		case kCubeBox:
			ioPt.x = RandomDbl(mCtr.x - mSideLen / 2, mCtr.x + mSideLen / 2);
			ioPt.y = RandomDbl(mCtr.y - mSideLen / 2, mCtr.y + mSideLen / 2);
			ioPt.z = RandomDbl(mCtr.z - mSideLen / 2, mCtr.z + mSideLen / 2);
		break;
		case kRPBox:
			ioPt.x = RandomDbl(mCtr.x - mSideLenX / 2, mCtr.x + mSideLenX / 2);
			ioPt.y = RandomDbl(mCtr.y - mSideLenY / 2, mCtr.y + mSideLenY / 2);
			ioPt.z = RandomDbl(mCtr.z - mSideLenZ / 2, mCtr.z + mSideLenZ / 2);
		break;
		case kCylBox:
			do {
				ioPt.x = RandomDbl(mCtr.x - mRadius, mCtr.x + mRadius);
				ioPt.y = RandomDbl(mCtr.y - mRadius, mCtr.y + mRadius);
				ioPt.z = RandomDbl(mCtr.z - mHeight / 2, mCtr.z + mHeight / 2);
			} while (!RawPointInBox(ioPt));
		break;
		case kSidesBox:
			do {
				ioPt = (GetExscribedBox())->RandPtInPrimitive();
			} while (!RawPointInBox(ioPt));
		break;
		default:
			throw;
		break;
	}
}

// ---------------------------------------------------------------------------
//		¥ RandPtInPrimitive
// ---------------------------------------------------------------------------
Point3DFloat &
SideSet::RandPtInPrimitive()
{
	static Point3DFloat outPt;
	switch (mType) {
		case kCubeBox:
			outPt.x = RandomDbl(mCtr.x - mSideLen / 2, mCtr.x + mSideLen / 2);
			outPt.y = RandomDbl(mCtr.y - mSideLen / 2, mCtr.y + mSideLen / 2);
			outPt.z = RandomDbl(mCtr.z - mSideLen / 2, mCtr.z + mSideLen / 2);
		break;
		case kRPBox:
			outPt.x = RandomDbl(mCtr.x - mSideLenX / 2, mCtr.x + mSideLenX / 2);
			outPt.y = RandomDbl(mCtr.y - mSideLenY / 2, mCtr.y + mSideLenY / 2);
			outPt.z = RandomDbl(mCtr.z - mSideLenZ / 2, mCtr.z + mSideLenZ / 2);
		break;
		case kCylBox:
			do {
				outPt.x = RandomDbl(mCtr.x - mRadius, mCtr.x + mRadius);
				outPt.y = RandomDbl(mCtr.y - mRadius, mCtr.y + mRadius);
				outPt.z = RandomDbl(mCtr.z - mHeight / 2, mCtr.z + mHeight / 2);
			} while (!RawPointInBox(outPt));
		break;
		case kSidesBox:
			do {
				outPt = (GetExscribedBox())->RandPtInPrimitive();
			} while (!RawPointInBox(outPt));
		break;
		default:
			throw;
		break;
	}
	return outPt;
}
*/

// ---------------------------------------------------------------------------
//		¥ RandPtInPrimitive(nuclProbType *nuclProb)
// ---------------------------------------------------------------------------
Point3DFloat &
SideSet::RandPtInPrimitive(nuclProbType	*nuclProb)
{
	static Point3DFloat outPt;
	float likelihood, actual;
	if (nuclProb != nil) {
		switch (mType) {
			case kCubeBox:
				do {
					outPt.x = RandomDbl(mCtr.x - mSideLen / 2, mCtr.x + mSideLen / 2);
					outPt.y = RandomDbl(mCtr.y - mSideLen / 2, mCtr.y + mSideLen / 2);
					outPt.z = RandomDbl(mCtr.z - mSideLen / 2, mCtr.z + mSideLen / 2);
					likelihood =  Calc1Probability(nuclProb->x, (outPt.x + (mCtr.x - mSideLen / 2)) / mSideLen)
								* Calc1Probability(nuclProb->y, (outPt.y + (mCtr.y - mSideLen / 2)) / mSideLen)
								* Calc1Probability(nuclProb->z, (outPt.z + (mCtr.z - mSideLen / 2)) / mSideLen);
					actual = RandomDbl(0, 1.0);
				} while (likelihood < actual);
			break;
			case kRPBox:
				do {
					outPt.x = RandomDbl(mCtr.x - mSideLenX / 2, mCtr.x + mSideLenX / 2);
					outPt.y = RandomDbl(mCtr.y - mSideLenY / 2, mCtr.y + mSideLenY / 2);
					outPt.z = RandomDbl(mCtr.z - mSideLenZ / 2, mCtr.z + mSideLenZ / 2);
					likelihood =  Calc1Probability(nuclProb->x, (outPt.x + (mCtr.x - mSideLenX / 2)) / mSideLenX)
								* Calc1Probability(nuclProb->y, (outPt.y + (mCtr.y - mSideLenY / 2)) / mSideLenY)
								* Calc1Probability(nuclProb->z, (outPt.z + (mCtr.z - mSideLenZ / 2)) / mSideLenZ);
					actual = RandomDbl(0, 1.0);
				} while (likelihood < actual);
			break;
			case kCylBox:
				throw("Can't do RandPtInPrimitive with PDF on a kCylBox!");
			break;
			case kSidesBox:
				throw("Can't do RandPtInPrimitive with PDF on a kSidesBox!");
			break;
			default:
				throw;
			break;
		}
	} else { // then nuclProb == nil
		switch (mType) {
			case kCubeBox:
				outPt.x = RandomDbl(mCtr.x - mSideLen / 2, mCtr.x + mSideLen / 2);
				outPt.y = RandomDbl(mCtr.y - mSideLen / 2, mCtr.y + mSideLen / 2);
				outPt.z = RandomDbl(mCtr.z - mSideLen / 2, mCtr.z + mSideLen / 2);
				break;
			case kRPBox:
				outPt.x = RandomDbl(mCtr.x - mSideLenX / 2, mCtr.x + mSideLenX / 2);
				outPt.y = RandomDbl(mCtr.y - mSideLenY / 2, mCtr.y + mSideLenY / 2);
				outPt.z = RandomDbl(mCtr.z - mSideLenZ / 2, mCtr.z + mSideLenZ / 2);
				break;
			case kCylBox:
				do {
					outPt.x = RandomDbl(mCtr.x - mRadius, mCtr.x + mRadius);
					outPt.y = RandomDbl(mCtr.y - mRadius, mCtr.y + mRadius);
					outPt.z = RandomDbl(mCtr.z - mHeight / 2, mCtr.z + mHeight / 2);
				} while (!RawPointInBox(outPt));
				break;
			case kSidesBox:
				do {
					outPt = (GetExscribedBox())->RandPtInPrimitive();
				} while (!RawPointInBox(outPt));
				break;
			default:
				throw;
				break;
		}
	}
	return outPt;
}

// ---------------------------------------------------------------------------
//		¥ Calc1Probability(nuclProbType *nuclProb)
// ---------------------------------------------------------------------------
/*CalcOneProbability -- This routine calculates the value of one sine curve, varying from fMinProb to 1,
	starting at angle fStartAngle, consisting of fHalfSines 180 degree intervals over the range [0,1] 
	at fVal.  Sorry for the long sentence.

	fScaleFactor: 	The scaling factor to transform the sine curve range from [-1,1] to [fMinProb,1].
	fStartRad:		The starting angle in radians (it is passed in as degrees).
	fAngle: 		The angle at fVal over the interval [0,1].
	inLoc:			The value, passed in as a fraction in the interval [0,1]. 
*/
double
SideSet::Calc1Probability(nuclProb1D &nuclProb, float inLoc)
{
	double ScaleFactor = (1.0 - nuclProb.minProb) / 2.0;
	double StartRad = (nuclProb.start / 180.0) * M_PI;
	double Angle = nuclProb.halfSines * M_PI * inLoc - StartRad;
//	double dummy = (sin(Angle) + 1) * ScaleFactor + nuclProb.minProb;	// for debugging
	return (sin(Angle) + 1) * ScaleFactor + nuclProb.minProb;
}
