// =================================================================================
//	CrystalArray.cp 						
// =================================================================================

#import "CrystalArray.h"
#import "SideSet.h"
#import <string>
#import <algorithm>
#import "stringFile.h"
#import "Calculator.h"

// ---------------------------------------------------------------------------
//		¥ CrystalArray
// ---------------------------------------------------------------------------
//	Constructor for an empty CrystalArray

CrystalArray::CrystalArray(Calculator *inCalc, long inNumExpected)
{
	mCalc = inCalc;
	mPrefs = mCalc->getPrefs();
	mNegRadii = 0;
	mNuclProb = nil;
	mListIsSorted = false;

	// reserve space for the array
	array.reserve(inNumExpected);
	mList.reserve(inNumExpected);
}

// ---------------------------------------------------------------------------
//		¥ CrystalArray(CrystalArray &inXls)
// ---------------------------------------------------------------------------
CrystalArray::CrystalArray(Calculator *inCalc, CrystalArray &inXls)
{
	mCalc = inCalc;
	mNuclProb = nil;
	mListIsSorted = false;
	InitXlArray(&inXls);
}


// ---------------------------------------------------------------------------
//		¥ CrystalArray(CrystalArray *inXls)
// ---------------------------------------------------------------------------
CrystalArray::CrystalArray(Calculator *inCalc, CrystalArray *inXls)
{
	mCalc = inCalc;
	mNuclProb = nil;
	mListIsSorted = false;
	InitXlArray(inXls);
}

// ---------------------------------------------------------------------------
//		¥ ~CrystalArray
// ---------------------------------------------------------------------------
//	Destructor for CrystalArray

CrystalArray::~CrystalArray()
{
	if (mNuclProb != nil) {
		delete mNuclProb;
		mNuclProb = nil;
	}
	Clear();
	
}

// ---------------------------------------------------------------------------
//		¥ Copy(CrystalArray *inXls)
// ---------------------------------------------------------------------------
void
CrystalArray::Copy(CrystalArray *inXls)
{
	Clear();
	mFile = inXls->mFile;
	mIntComment = inXls->mIntComment;
	mTotalVolume = inXls->mTotalVolume;
	mBounds = inXls->mBounds;
	mNegRadii = inXls->mNegRadii;
	mLower = inXls->mLower;
	mUpper = inXls->mUpper;
	mCtr = inXls->mCtr;
	mRadius = inXls->mRadius;
	mHeight = inXls->mHeight;
	mNumCrystals = inXls->mNumCrystals;
	if (mNuclProb) {
		delete mNuclProb;
		mNuclProb = nil;
	}
	if (inXls->mNuclProb)
		mNuclProb = new nuclProbType(*(inXls->mNuclProb));
	
	mList = inXls->mList;
	
	array = inXls->array;
}

// ---------------------------------------------------------------------------
//		¥ InitXlArray(CrystalArray *inXls)
//	This initializes the Crystal Array based on the general parameters of another,
//	but does not copy the actual crystals.
// ---------------------------------------------------------------------------
void
CrystalArray::InitXlArray(CrystalArray *inXls)
{
	mFile = inXls->mFile;
	mIntComment = inXls->mIntComment;
	mTotalVolume = inXls->mTotalVolume;
	mBounds = inXls->mBounds;
	mNegRadii = inXls->mNegRadii;
	mLower = inXls->mLower;
	mUpper = inXls->mUpper;
	if (inXls->mNuclProb)
		mNuclProb = new nuclProbType(*(inXls->mNuclProb));
}

#pragma mark Array Methods
// ---------------------------------------------------------------------------------
//		¥ PopXl
// ---------------------------------------------------------------------------------
Crystal
CrystalArray::PopXl(bool inKeepListCorrect)
{
	static Crystal outXl;
	
	if (array.size() > 0) {
		outXl = array.back();
		array.pop_back();
	} else {		
		throw ArrayIOErr();
	}
	
	if (inKeepListCorrect) {
		// construct an XListElement
		XListElement temp;
		temp.index = array.size();		// we want the previous index of the popped Crystal (we reduced the size
										// in the pop_back() call above.  E.g.:If we started with 4 elements, then popped one
										// then the index of the popped one was 3 (zero-based array).  The current size 
										// (after popping) is also 3
		temp.location = outXl.ctr.x;
		std::vector<XListElement>::iterator it;
		it = find (mList.begin(), mList.end(), temp);
		mList.erase(it);
	}
	
	return outXl;
}

// ---------------------------------------------------------------------------------
//		¥ PushXl
// ---------------------------------------------------------------------------------
void
CrystalArray::PushXl(Crystal &inXl, bool inKeepListCorrect)
{
	array.push_back(inXl);

	if (inKeepListCorrect) {
		// construct an XListElement
		XListElement temp;
		temp.index = array.size() - 1;	// If the size was 3, and we just added one, then the current size is 4, and the index
										// of the just-pushed Crystal is 3 (zero-based array)
		temp.location = inXl.ctr.x;
		std::vector<XListElement>::iterator it;
		it=lower_bound (mList.begin(), mList.end(), temp);
		it = mList.insert (it, temp);
	}
}


// ---------------------------------------------------------------------------------
//		¥ operator[]
// ---------------------------------------------------------------------------------
Crystal *
CrystalArray::operator[](int inSub)
{
	return &(array.at(inSub)); 	
}

// ---------------------------------------------------------------------------------
//		¥ GetItemPtr
// ---------------------------------------------------------------------------------
Crystal *
CrystalArray::GetItemPtr(int i)
{
	return &(array.at(i));
}

// ---------------------------------------------------------------------------------
//		¥ Clear
// ---------------------------------------------------------------------------------
void
CrystalArray::Clear()
{
	array.clear();
	mList.clear();
	mTotalVolume = 0;
	mNegRadii = 0;
}

// ---------------------------------------------------------------------------
//		¥ UpdateXl
// ---------------------------------------------------------------------------
void
CrystalArray::UpdateXl(int index, Crystal *inXl, bool inKeepListCorrect)
{
	if (inKeepListCorrect) {
		array.at(index) = *inXl;
	} else {
		RemoveItem (index, true);
		PushXl(*inXl, true);
	}
}

// ---------------------------------------------------------------------------------
//		¥ RemoveItem
// ---------------------------------------------------------------------------------
void
CrystalArray::RemoveItem(int inAtIndex, bool inKeepListCorrect)
{
	Crystal *deadXl = nil;

	if (inKeepListCorrect) {
		deadXl = GetItemPtr(inAtIndex);
		XListElement temp;
		temp.index = inAtIndex;
		temp.location = deadXl->ctr.x;
		std::vector<XListElement>::iterator it;
		it = find (mList.begin(), mList.end(), temp);
		mList.erase(it);
	}

	array.erase(array.begin() + (inAtIndex));
}


#pragma mark File Methods

// ---------------------------------------------------------------------------
//		¥ ExtractDouble
//	This utility function (not a class method) takes a string (STL) and parses a
//	substring for a double.
// ---------------------------------------------------------------------------
double
ExtractDouble(const std::string inStr, short from, short to)
{
	std::string temp = inStr.substr(from, to-from);
	double outVal;
	sscanf(temp.c_str(), "%lf", &outVal);
	return outVal;
}

// ---------------------------------------------------------------------------
//		¥ ReadMergeHeader
// ---------------------------------------------------------------------------
void
CrystalArray::ReadMergeHeader()
{
	std::string	tempStr;
	std::string	tempStr2;
	int sscanResult;
	try {
		// read header
		tempStr = mFile->getOneLine();		// Throw away Integrate header line (gives the version of the file, but this isn't used much)
		mIntComment = mFile->getOneLine();		// Comment line, which for files made by Crystallize, contains run parameters
		tempStr2 = mFile->peekLine();		// Probably the Number of crystals line, but some programs create a second comment line here.
											//   we'll look for an "N" as the first character to detect a non - Number-of-crystals line here.
		if (tempStr2[0] != 'N') {
			tempStr2 = mFile->getOneLine();		// Secondary comment line 
			mIntComment += tempStr2;		// append secondary comment line to the comment variable
		}
		
		// Figure out what produced the input file, based on what's in the comment line.
		//	In order to detect Crystallize 2 files, it looks for the string ", Qd = ", which
		//	is (at this point) unique to these simulations.  A hack, I know.  Otherwise, if
		//	the key words "diffusion", "interface" or "heat flow" appear in the comment line,
		//	the file is taken to be a Crystallize 1 simulation of the appropriate type.  If
		//	none of these things hold true, it is assumed to be real data.
		if ((mIntComment.find("Qd") != std::string::npos) || (mIntComment.find("diffusion") != std::string::npos))
			mFileType = kDiffSimulation;
		else if (mIntComment.find("interface") != std::string::npos)
			mFileType = kIntSimulation;
		else if (mIntComment.find("heat flow") != std::string::npos)
			mFileType = kHFSimulation;
		else
			mFileType = kReal;
		
		// parse comment for probability data
		// is there a pdf?
		short paramLoc = mIntComment.find("pdf");
		if (paramLoc && mPrefs->matchPDF) {
			mNuclProb = new nuclProbType;
			while (!ISNUMCHAR((char)mIntComment[paramLoc])) {
				paramLoc++;
			}
			short nextComma = mIntComment.find(',', paramLoc);
			mNuclProb->x.halfSines = (short) ExtractDouble(mIntComment, paramLoc, nextComma);
			paramLoc = nextComma+1;
			nextComma = mIntComment.find(',', paramLoc);
			mNuclProb->x.start = (short) ExtractDouble(mIntComment, paramLoc, nextComma);
			paramLoc = nextComma+1;
			nextComma = mIntComment.find(',', paramLoc);
			mNuclProb->x.minProb = ExtractDouble(mIntComment, paramLoc, nextComma);
			paramLoc = nextComma+1;
			nextComma = mIntComment.find(',', paramLoc);
			mNuclProb->y.halfSines = (short) ExtractDouble(mIntComment, paramLoc, nextComma);
			paramLoc = nextComma+1;
			nextComma = mIntComment.find(',', paramLoc);
			mNuclProb->y.start = (short) ExtractDouble(mIntComment, paramLoc, nextComma);
			paramLoc = nextComma+1;
			nextComma = mIntComment.find(',', paramLoc);
			mNuclProb->y.minProb = ExtractDouble(mIntComment, paramLoc, nextComma);
			paramLoc = nextComma+1;
			nextComma = mIntComment.find(',', paramLoc);
			mNuclProb->z.halfSines = (short) ExtractDouble(mIntComment, paramLoc, nextComma);
			paramLoc = nextComma+1;
			nextComma = mIntComment.find(',', paramLoc);
			mNuclProb->z.start = (short) ExtractDouble(mIntComment, paramLoc, nextComma);
			paramLoc = nextComma+1;
			mNuclProb->z.minProb = ExtractDouble(mIntComment, paramLoc, nextComma);
		} else {
			mNuclProb = nil;
		}
		tempStr = mFile->getOneLine();		// This line ought to be the Number of crystals line 
		if (tempStr[0] != 'N') {			// If it doesn't begin with 'N', then it wasn't
			tempStr = mFile->getOneLine();	// and the next one must be the Number of crystals line 	
		}
		// Parse for the number of crystals:
		// If no tab is present, then it's the old style of Integrate file, in which the values are organized based on their
		// horizontal location on the line.
		if (tempStr.find('\t') == std::string::npos)	
			sscanResult = sscanf(tempStr.c_str(), "%*25c%i", &mNumCrystals);
		else
			sscanResult = sscanf(tempStr.substr(tempStr.find('\t'), std::string::npos).c_str(), "%i", &mNumCrystals);
		if (sscanResult != 1) throw(MergeIOErr());
		
		tempStr = mFile->getOneLine();		// Total volume line 
		if (tempStr.find('\t') == std::string::npos)	// no tab present
			sscanResult = sscanf(tempStr.c_str(), "%*25c%f", &mTotalVolume);
		else
			sscanResult = sscanf(tempStr.substr(tempStr.find('\t'), std::string::npos).c_str(), "%f", &mTotalVolume);
		if (sscanResult != 1) throw(MergeIOErr());
		
		tempStr = mFile->peekLine();		// Bounds line 
		if (tempStr.find("Bound") != std::string::npos) {	// "Bound" present (RP)
			tempStr = mFile->getOneLine();		// Bounds line (RP) 
			sscanf(tempStr.c_str(), "Bounds:\t%f%f%f%f%f%f", &(mLower.x), &(mLower.y), &(mLower.z),
				   &(mUpper.x), &(mUpper.y), &(mUpper.z));
			mBounds = kBoundsRP;
		} else if (tempStr.find("Ctr/R/H") != std::string::npos) {
			tempStr = mFile->getOneLine();		// Bounds line (Cyl)
			sscanf(tempStr.c_str(), "Ctr/R/H:\t%f%f%f%f%f", &(mCtr.x), &(mCtr.y), &(mCtr.z),
				   &(mRadius), &(mHeight));
			mBounds = kBoundsCyl;
		} else if (isSim(mFileType)) {	// then it's a simulation with volume=mTotalVolume, so we know the bounds
			mBounds = kBoundsRP;
			mLower.Set(0,0,0);
			float dimension = CubeRoot(mTotalVolume);
			mUpper.Set(dimension,dimension,dimension);
		} else  {
			mBounds = kBoundsNone;
		}
	} catch (stringFile::UnexpectedEOF sfErr) {
		MergeIOErr err;
		err.briefDesc = "Unexpected End of File";
		char tempLongDesc[kStdStringSize];
		sprintf(tempLongDesc, "Problem reading header data from file: %s.", sfErr.filename);
		err.longDesc = tempLongDesc;
		mCalc->postError(err.longDesc.c_str(), err.briefDesc.c_str(), nil, -1, -1);
		throw;	// toss it up the chain
	}
}



 // ---------------------------------------------------------------------------
 //		¥ ReadMergeFile
 // ---------------------------------------------------------------------------
 void
 CrystalArray::ReadMergeFile()
 {
	 std::string	tempStr;
	 short	checkCrystalNum;
	 short	curCrystal;
	 Crystal	oneCrystal;
	 
	 ReadMergeHeader();
	 mCalc->setupProgress("Reading Crystals...", nil, nil, nil, -1, 1, mNumCrystals, 0, false);	 

	 // Note that in the file, the crystals are numbered beginning at 1, but in all the arrays, the
	 // numbering is zero-based	 
	 for (curCrystal = 1; curCrystal <= mNumCrystals; curCrystal++) {
		 try {
			 mCalc->progress(curCrystal);
			 if (mCalc->shouldStopCalculating()) throw(kUserCanceledErr);
			 tempStr = mFile->getOneLine();	// if this tries to get a line past the end (e.g., if the number of crystals claimed in
											// the header was false), then it will throw an exception.
		 } catch (stringFile::UnexpectedEOF sfErr) {
			 if (curCrystal < mNumCrystals)	{ // hit EOF before last line - bad
				 MergeIOErr err;
				 char xlNumStr[15];
				 sprintf(xlNumStr, "%d", curCrystal-1);
				 err.briefDesc = "Unexpected End of File";
				 char tempLongDesc[kStdStringSize];
				 sprintf(tempLongDesc, "Problem reading crystals from file: %s. Last crystal read and verified was: %d.",
						 sfErr.filename, 
						 curCrystal-1);	// This reported crystal number is the one from the file (1-based)
				 err.longDesc = tempLongDesc;
				 mCalc->postError(err.longDesc.c_str(), err.briefDesc.c_str(), nil, -1, -1);
				 throw;	// toss it up the chain
			 } // else: ignore the problem; we've got all the crystals we need
		 }
		 if (tempStr.find('\t') == std::string::npos) {	// no tab present
			 sscanf(tempStr.c_str(), "%hi%f%f%f%f%hi%li", 
					&checkCrystalNum,
					&oneCrystal.ctr.x,
					&oneCrystal.ctr.y,
					&oneCrystal.ctr.z,
					&oneCrystal.r,
					&oneCrystal.ctrSlice,
					&oneCrystal.ctrID);
		 } else {
			 sscanf(tempStr.c_str(), "%hi\t%f\t%f\t%f\t%f\t%hi\t%li", 
					&checkCrystalNum,
					&oneCrystal.ctr.x,
					&oneCrystal.ctr.y,
					&oneCrystal.ctr.z,
					&oneCrystal.r,
					&oneCrystal.ctrSlice,
					&oneCrystal.ctrID);
		 }
		 if (oneCrystal.r == 0)
			 continue;	// run next for() loop
		 PushXl(oneCrystal, false);
		 if (checkCrystalNum != curCrystal)  {	// this crystal number isn't the one we expected next 
			 BadOrderErr thisErr;
			 thisErr.expectedNum = curCrystal;
			 thisErr.foundNum = checkCrystalNum;
			 throw thisErr;
		 }
	 } // end for
	 SortInternalList();
 }
 
// ---------------------------------------------------------------------------------
//		¥ CrystalIntersects
// ---------------------------------------------------------------------------------
//  This function is called when trying to place a crystal in a volume as part of an
//	envelope simulation.  We need to know if a random location for a given crystal
//	is legal given the run settings.
//
//	Although this returns a short, it is meant to be used as a bool as well: a return
//	value > 0 means true (it does intersect).  The additional values are meant to
//	give information as to what criterion caused the intersection.
//
//	The Interface-control placement criterion is:
//		(a)  The later-nucleated crystal cannot have nucleated within the volume of
//				the earlier-nucleated one as that one existed at the time of the nucleation
//				of the later-nucleated one.
//		Assuming interface-controlled growth, all crystals grow as r=kt, so
//				radius increments are proportional to time.  
//
// this is the easy-to-read but slower version:
//			double rMaxAtTauOne = bigXl->r - smallXl->r;	// the radius of the largest Xl at Tau = 1
//			double tauNucl = (rMaxAtTauOne - (thisXl.r - smallXl->r)) / rMaxAtTauOne;	// the tau at which cur Xl would've nucleated
//			double otherRadAtNucl = otherXl->r - smallXl->r - (rMaxAtTauOne * (1 - tauNucl));	// the radius the other crystal had when cur Xl nucleated
//			double overlapAtNucl = otherRadAtNucl + 0 - otherXl->ctr.Distance(thisXl.ctr);
//			if (overlapAtNucl > 0)
//				return true;

short
CrystalArray::CrystalIntersects(Crystal &thisXl, bool inMatchingCTDataSet, float inVolFraction, float betafactorfactor)
{
	float separation;
	if (GetNumXls() == 0)
		return false;

	Crystal *otherXl;
	int numXls = GetNumXls();
	
	if (mPrefs->observabilityFilter && inMatchingCTDataSet) {
		for (int i = 0; i <= numXls-1; i++) {
			otherXl = (Crystal *) GetItemPtr(i);
			separation = thisXl.ctr.Distance(otherXl->ctr);
			float minOKDist;
			if (mPrefs->makeDCEnv) {	// if we are making a "diffusion-controlled" envelope as in Chris Daniel's dissertation
				float betaFactor = pow(inVolFraction, -1.0/3.0);
				minOKDist = betaFactor * fabs(otherXl->r - thisXl.r) * betafactorfactor;
				if (minOKDist >= separation) {
					return kIC_or_DCCriterion;
				}
			} else {
				minOKDist = fabs(otherXl->r - thisXl.r);
				if (separation < (thisXl.r + otherXl->r))	{	// if they overlap at all, then do placement checks:
					// IC Criterion:
					if (minOKDist >= separation) {
						return kIC_or_DCCriterion;
					}
					// is this pair observable?
					Crystal *smaller, *larger;
					if (thisXl.r < otherXl->r) {
						smaller = &thisXl;
						larger = otherXl;
					} else {
						smaller = otherXl;
						larger = &thisXl;
					}
					float IntPlaneDist = (sqr(larger->r) - sqr(smaller->r) + sqr(separation)) / (2 * separation);
					if (separation < mPrefs->crit1Factor * IntPlaneDist) {	// criterion (1)
						return kObs1Criterion;
					} else {
						float totalLength = separation + larger->r + smaller->r;
						if (totalLength < mPrefs->crit2Factor * smaller->r) {	// criterion (2)
							return kObs2Criterion;
						}
					}
				}
			}
		}
	} else {
		for (int i = 0; i <= numXls - 1; i++) {
			otherXl = (Crystal *) GetItemPtr(i);
			separation = thisXl.ctr.Distance(otherXl->ctr);
			// this is the less obvious, but faster(?) version:
			float minOKDist;
			if (mPrefs->makeDCEnv) {
				float betaFactor = pow(inVolFraction, -1.0/3.0);
				minOKDist = betaFactor * fabs(otherXl->r - thisXl.r) * betafactorfactor;
			} else {
				minOKDist = fabs(otherXl->r - thisXl.r);
			}
			if (minOKDist >= separation) {
				return kIC_or_DCCriterion;
			}
		}
	}

	return kNoIntersection;
}

// ---------------------------------------------------------------------------------
//		¥ FindCrystalAt
// ---------------------------------------------------------------------------------
bool
CrystalArray::FindCrystalAt(Point3DFloat inPt, Crystal *thisXl)
{
	Crystal *curXl;
	bool found = false;
	int numXls = GetNumXls();

	for (int i=0; (i <= numXls-1) && (!found); i++) {
		curXl = (Crystal *) GetItemPtr(i);
		if (curXl->ctr == inPt) {
			found = true;
			*thisXl = *curXl;
		}
	}
	return found;
}

// ---------------------------------------------------------------------------------
//		¥ Average
// ---------------------------------------------------------------------------------
Point3DFloat &
CrystalArray::Average()
{
	Crystal *thisXl;
	int numXls = GetNumXls();
	static Point3DFloat outPt;
	for (int i=0; i <= numXls - 1; i++) {
		thisXl = (Crystal *) GetItemPtr(i);
		outPt += thisXl->ctr;
	}
	outPt /= numXls;
	return outPt;
}

// ---------------------------------------------------------------------------------
//		¥ GetMeanRadius
// ---------------------------------------------------------------------------------
double
CrystalArray::GetMeanRadius()
{
	double rsum = 0;
	Crystal *thisXl;
	int numXls = GetNumXls();
	for (int i=0; i <= numXls - 1; i++) {
		thisXl = (Crystal *) GetItemPtr(i);
		rsum += thisXl->r;
	}
	
	return rsum / numXls;
}


// ---------------------------------------------------------------------------
//		¥ FilterForObservability
// ---------------------------------------------------------------------------
/*	This function examines each pair of crystals to see if they would be observable
in a real dataset.  If not, the pair are combined into one with a slightly larger
volume, the idea being that this is what a human operator would interpret if the
crystals wrere in a CT dataset.
There are two criteria; either will make the pair of crystals be unobservable.
(1)  The center of the smaller sphere is at least 10% closer to the center of the larger sphere
than is the plane of intersection of the sphere surfaces.
(2)  The total length of the pair is less than 3 times the radius of the smaller sphere.
** Currently, clusters of >1 crystal are dealt with by examining the set pairwise
*/
void
CrystalArray::FilterForObservability()
{
	bool unobservable;
	
/*	myLFileStream *debugOutputFile;
	std::string debugStr;
	if (mPrefs->verbose) {
		CReduceApp *theApp = (CReduceApp *) LCommander::GetTopCommander();
		debugOutputFile = theApp->GetDebugFile();
		debugOutputFile->OpenDataFork(fsRdWrPerm);
		debugStr = "Beginning Observability Filter\n";
		debugOutputFile->SetMarker(0, streamFrom_End);
		debugOutputFile->putOneLine(debugStr);
		debugOutputFile->CloseDataFork();
	}
*/	
	Crystal *thisXl, *otherXl;
	for (int i = 0; i <= GetNumXls()-1; i++) {
		thisXl = (Crystal *) GetItemPtr(i);
		for (int j=i+1; j <= GetNumXls()-1; j++) {
			otherXl = (Crystal *) GetItemPtr(j);
			float separation = thisXl->ctr.Distance(otherXl->ctr);
			if (separation < (thisXl->r + otherXl->r))	{	// if they overlap at all, then check for observability:
															// is this pair observable?
				unobservable = false;
				Crystal *smaller, *larger;
				if (thisXl->r < otherXl->r) {
					smaller = thisXl;
					larger = otherXl;
				} else {
					smaller = otherXl;
					larger = thisXl;
				}
				float IntPlaneDist = (sqr(larger->r) - sqr(smaller->r) + sqr(separation)) / (2 * separation);
				if (separation < mPrefs->crit1Factor * IntPlaneDist) {	// criterion (1)
					unobservable = 1;
				} else {
					float totalLength = separation + larger->r + smaller->r;
					if (totalLength < mPrefs->crit2Factor * smaller->r) {	// criterion (2)
						unobservable = 2;
					}
				}
				
				if (unobservable) {	// then we need to combine them as a human would in fuzzy CT data
					Crystal newXl;
					// put center at combined center-of-mass
					float smallVol = (4.0 * pi * pow(smaller->r, 3) / 3.0);
					float largeVol = (4.0 * pi * pow(larger->r, 3) / 3.0);
					newXl.ctr = (larger->ctr * (largeVol/(largeVol + smallVol)));
					newXl.ctr += (smaller->ctr * (smallVol/(largeVol + smallVol)));
					// make volume (via radius) be combined volume: we need to add the two volumes, and subtract
					// the spherical caps of each one that form the intersection volume (spherical cap
					// volume formula is taken from pg. 314 of CRC Math Tables, 30th Ed.)
					float h = larger->r - IntPlaneDist;
					float largeCapVol = pi * sqr(h) * (3.0 * larger->r - h) / 3.0;
					h = smaller->r - separation + IntPlaneDist;
					float smallCapVol = pi * sqr(h) * (3.0 * smaller->r - h) / 3.0;
					float totalVol = smallVol + largeVol - largeCapVol - smallCapVol;
					newXl.r = CubeRoot(0.75 * totalVol / pi);
					
/*					if (mPrefs->verbose) {
						CReduceApp *theApp = (CReduceApp *) LCommander::GetTopCommander();
						myLFileStream *debugOutputFile = theApp->GetDebugFile();
						debugOutputFile->OpenDataFork(fsRdWrPerm);
						debugStr = "Tossed out a pair dur to criterion #";
						debugStr += (SInt32) unobservable;
						debugStr += "; numbers were ";
						debugStr += (SInt32) i;
						debugStr += " and ";
						debugStr += (SInt32) j;
						debugStr += ", and added a new one, number ";
						debugStr += (SInt32) GetNumXls() - 1;
						debugStr += "\n";
						debugOutputFile->SetMarker(0, streamFrom_End);
						debugOutputFile->putOneLine(debugStr);
						debugOutputFile->CloseDataFork();
					}
*/
					RemoveItem(j, false);
					RemoveItem(i, false);
					PushXl(newXl, false);
					
					// force routine to redo this "i" loop by breaking out of j loop and
					// decrementing i so it will be incremented to the current value the
					// next time the for is executed.
					j = GetNumXls();
					i--;
				}
			}
		}
	}
	RebuildList();
}

// ---------------------------------------------------------------------------
//		¥ GetNuclProb
// ---------------------------------------------------------------------------
nuclProbType*
CrystalArray::GetNuclProb()
{
	return mNuclProb;
}

//Not called
// ---------------------------------------------------------------------------------
//		¥ ChangeRadii
// ---------------------------------------------------------------------------------
/*
void
CrystalArray::ChangeRadii(double *outRadiiList, float inDelta)
{
	Crystal *thisXl;
	int numXls = GetNumXls();
	for (int i=0; i <= numXls - 1; i++) {
		thisXl = (Crystal *) GetItemPtr(i);
		outRadiiList[i] = thisXl->r + inDelta;
	}
}
*/

/* NOT ACTUALLY CALLED.  I think this was once a way to make the simulations obey the IC criterion,
but it's bogus, so we just pick a new random placing if a location is bad 
// ---------------------------------------------------------------------------------
//		¥ AdjustLocations
// ---------------------------------------------------------------------------------
bool
CrystalArray::AdjustLocations()
{
	Crystal *thisXl;
	Lock();
	bool neededAdjustment = false;
	for (short i=1; i <= GetNumXls() - 1; i++) {
		thisXl = (Crystal *) GetItemPtr(i);
		// test for intersections
		Crystal *otherXl;
		for (short j=i+1; j <= GetNumXls(); j++) {
			otherXl = (Crystal *) GetItemPtr(j);
				// This is the Interface-Controlled placing criterion:

// this is the easy-to-read but slower version:
//			double rMaxAtTauOne = bigXl->r - smallXl->r;	// the radius of the largest Xl at Tau = 1
//			double tauNucl = (rMaxAtTauOne - (thisXl.r - smallXl->r)) / rMaxAtTauOne;	// the tau at which cur Xl would've nucleated
//			double otherRadAtNucl = otherXl->r - smallXl->r - (rMaxAtTauOne * (1 - tauNucl));	// the radius the other crystal had when cur Xl nucleated
//			double overlapAtNucl = otherRadAtNucl + 0 - otherXl->ctr.Distance(thisXl.ctr);
//			if (overlapAtNucl > 0)
//				return true;

// this is the less obvious, but faster(?) version:
			if ((otherXl->r - thisXl->r - otherXl->ctr.Distance(thisXl->ctr)) > 0) {
				neededAdjustment = true;
				// too much overlap, so move the later-nucleated (smaller) one away
				Crystal *moveXl;
				Crystal *holdXl;
				if (thisXl->r > otherXl->r) {
					moveXl = otherXl;
					holdXl = thisXl;
				} else {
					holdXl = otherXl;
					moveXl = thisXl;
				}
				Point3DFloat originalLoc = moveXl->ctr;
				// try moving it
				Point3DFloat newCtr = ((moveXl->ctr - holdXl->ctr).Unit()) * (holdXl->r - moveXl->r) + holdXl->ctr;
				moveXl->ctr = newCtr;
				
				// see if it now has too much overlap with any others
				Crystal *thirdXl;
				CrystalArray *overlapXls = nil;
				float overlap = 0;
				for (short k=1; k <= GetNumXls(); k++) {
					thirdXl = (Crystal *) GetItemPtr(k);
					if ((moveXl->r - thirdXl->r - moveXl->ctr.Distance(thirdXl->ctr)) > 0) {
						overlap += moveXl->r - thirdXl->r - moveXl->ctr.Distance(thirdXl->ctr);
						if (overlapXls == nil) {
							overlapXls = new CrystalArray(GetNumXls());
						}
						overlapXls->PushXl(*thirdXl);
					}
				}
				
				if (overlap > 0) {	// if so, then negotiate the best location
					Point3DFloat direction = (moveXl->ctr - holdXl->ctr).Unit();
					float magnitude = (moveXl->ctr - holdXl->ctr).Magnitude();
					float factor = 1.0;
					bool done = false;
					float lastOverlap = overlap;
					float thisOverlap = 0;
					
					do {
						factor -= 0.1;
						moveXl->ctr = holdXl->ctr + direction * magnitude * factor;

						for (short l=1; l <= overlapXls->GetNumXls(); l++) {
							thirdXl = (Crystal *) overlapXls->GetItemPtr(l);
							if ((moveXl->r - thirdXl->r - moveXl->ctr.Distance(thirdXl->ctr)) > 0) {
								thisOverlap += moveXl->r - thirdXl->r - moveXl->ctr.Distance(thirdXl->ctr);
							}
						}
						
						if (thisOverlap > lastOverlap) {
							done = true;
							factor += 0.1;
							moveXl->ctr = holdXl->ctr + direction * magnitude * factor;
						}
					} while (!done);

					delete overlapXls;
				} else {
					// do nothing - we've got the right location.
				}
			}
		}
	}
	Unlock();
	return neededAdjustment;
}*/


// ---------------------------------------------------------------------------
//		¥ CorrectedCrystalVolume
// ---------------------------------------------------------------------------
/* This function returns the volume of a crystal, reduced by the amount that belongs to other crystals,
	as determined by the spherical cap on the far side of the plane of intersection with another crystal. 
	This is used by CorrectForImpingement to adjust the radius of crystals that are generated by Crystallize simulations.
*/
float
CrystalArray::CorrectedCrystalVolume (Crystal *inXl)
{
	int i;
	float volume;		// volume of the crystal -- total volume minus sectors belonging to other crystals
	float dist;			// distance between two sphere centers
	float sumRad;		// sum of two sphere radii
	float cosAlpha;		// two spheres will intersect in a circle.  Alpha is the angle made by a line going from the center
						// the currently considered crystal (inXl) to the center of the other crystal (thisXl) to a point on
						// the circle of intersection.  cosAlpha is the cosine of that angle.
	float h;			// distance between the dividing plane and the edge of the sphere whose volume we want
	float segmentVol;	// volume of the part of the sphere shaved off by the dividing plane
	float absXlRad, absCurrRad;	// Since radii are sometimes negative here for flagging purposes, these store Abs(radius)
	Crystal *thisXl;
	bool	dontbreak = true;
	int numXls = GetNumXls();
	
	absXlRad = fabs(inXl->r);
	volume = 4.0 * M_PI * absXlRad * absXlRad * absXlRad / 3.0;
	for (i = 0; (i <= numXls - 1) && dontbreak; i++) {
		thisXl = (Crystal *) GetItemPtr(i);
		if (thisXl != inXl) {
			absCurrRad = fabs(thisXl->r);
			dist = thisXl->ctr.Distance(inXl->ctr);
			sumRad = absXlRad + absCurrRad;
			if (sumRad > dist) {	// radii sum is longer than distance between centers -> spheres intersect
				if (dist + absXlRad < absCurrRad) {	// if this crystal is engulfed by the other
					volume = 0;
					dontbreak = false;  // Leave the "for i..." loop
				} else if (dist + absCurrRad > absXlRad) {	// if the other crystal is not engulfed by this one
					cosAlpha = (sqr(dist) + sqr(absXlRad) - sqr(absCurrRad)) / (2.0 * dist * absXlRad);	// law of cosines
//					cosAlpha = cosAlpha / (2.0 * dist * absXlRad);				// law of cosines, cont.
					if (cosAlpha > 0) {	// alpha is between 0-90 degrees
						h = absXlRad * (1.0 - cosAlpha);
						segmentVol = M_PI * h * h * (3.0 * absXlRad - h) / 3.0;	// volume of segment, from CRC handbook
						volume = volume - segmentVol;
					} else if (cosAlpha == 0) {	// alpha = 90 degrees
						volume = volume - 2.0 * M_PI * absXlRad * absXlRad * absXlRad / 3.0;
					} else {	// alpha is more than 90 degrees, so the center of the other crystal is too far inside this one
								// while the other crystal is not totally engulfed, its full radius is inside this crystal.
								// It's unclear how we can identify an amount of volume belonging to this other crystal.
								// We are essentially engulfing the other crystal, so we'll just do nothing here, and go onto
								// other crystals in the array
//						cosAlpha = cos(M_PI - acos(cosAlpha));	// recast alpha as 180 - alpha
//						h = absXlRad * (1.0 - cosAlpha);
//						segmentVol = M_PI * h * h * (3.0 * absXlRad - h) / 3.0;	// volume of segment, from CRC handbook
//						volume = volume - segmentVol;
					}
				}
			}
		}
	}
	if (volume < 0)
		volume = 0;
	return volume;
}


// ---------------------------------------------------------------------------
//		¥ CorrectForImpingement
// ---------------------------------------------------------------------------
/* CorrectForImpingement -- This routine nudges up the radius and extended volume for a crystal to compensate for impingement.
It is intended only to operate on datasets from Crystallize, not those from actual
crystals.  Crystallize doesn't address impingement at all, really, but rather takes the calculated domain volumes 
and multiplies them by the user-selected volume fraction, producing erroneously small radii when the domains impinge.
If we presume two spherical domains of radius R1 and R2 with the centers separated by distance d, then domain 1 has
a spherical cap volume CAP1 given by: CAP1 = pi*H1*H1*(3*R1-H1)/3, where H1 is the "height" of the cap and is given by:
H1 = R1 * (1 - (R1^2 + d^2 - R2^2) / (2 * R1 * d) )  (the derivation for this is difficult to explain in text, but relies on 
substantial trigonometry and the construction of two triangles).  The volume of the domain 1 that Crystallize actually knows about
is given by V1' = V1-CAP1, and the volume of crystal 1 in domain 1 is given by v1 = V1' * VF (where VF = volume fraction).
The radius recorded by Crystallize is merely r1 = (3 * v1 / (4 * pi)) ^ (1/3).  The true volume of the two domains
is JDV = V1' + V2', and the ideal joint volume of the two crystals would be v1 + v2.  However, if these crystals overlap,
then some of that joint volume will not be realized because the overlapped volume cannot be counted for both crystals.  Thus, we
add the overlapped volume back onto each crystal.  Of course, some of the added volume will also be overlapping, and thus we iterate
(making this routine perhaps take a while) until the percentage error is less than the global variable volumeErrorTolerance.	
*/
void
CrystalArray::CorrectForImpingement()
{
	bool maxErrorSmallEnough, meanErrorSmallEnough, notConverging;
	int	currXl, itNum, sign;
	float	volumeDifference, currErr, maxErr, avgErr, oldAvgErr, maxTol, meanTol,
		currVol;
	Crystal *thisXl;
	int numXls = GetNumXls();
	
	itNum = 0;
	oldAvgErr = 0;
	notConverging = false;
	maxTol = mPrefs->impingementMaxErr / 100;
	meanTol = mPrefs->impingementMeanErr / 100;
	do {
		maxErrorSmallEnough = true;
		itNum++;
		maxErr = 0.0;
		oldAvgErr = avgErr;
		avgErr = 0.0;
		for (currXl = 0; currXl <= numXls - 1; currXl++) {
			thisXl = (Crystal *) GetItemPtr(currXl);
			currVol = CorrectedCrystalVolume(thisXl);	// this is the volume reduced by the amount that belongs to other crystals
			if (currVol > 0) {	// ignore engulfed crystals
				volumeDifference = thisXl->actV - currVol;	// volume difference is the difference between the volume recorded by Crystallize (in the radius)
															// and the value of the extended volume after removing portions belonging to other crystals.
				thisXl->extV = volumeDifference + thisXl->extV; // here we increase the volume by the overlap amount
				currErr = volumeDifference / thisXl->actV;	// this is the volume reduction (in this iteration) as a fraction of the original recorded volume
				avgErr += currErr;
				if (currErr > maxErr)
					maxErr = currErr;
				if (currErr > maxTol)
					maxErrorSmallEnough = false;
			}
		}
		
		for (currXl = 0; currXl <= numXls - 1; currXl++) {
			thisXl = (Crystal *) GetItemPtr(currXl);
			if (thisXl->r < 0)
				sign = -1;
			else
				sign = 1;
			thisXl->r = sign * CubeRoot(thisXl->extV * 3 / (4 * M_PI));
		}
		
		avgErr = avgErr / numXls;
		if (avgErr <= meanTol) {
			meanErrorSmallEnough = true;
		} else {
			meanErrorSmallEnough = false;
		}
		
		if ((itNum > 1) && (avgErr > oldAvgErr))
			notConverging = true;
		
	} while (!(notConverging || (maxErrorSmallEnough && meanErrorSmallEnough)));
}



// ---------------------------------------------------------------------------
//		¥ RemoveIllegalOverlaps
// ---------------------------------------------------------------------------
/*
	** Currently, clusters of >1 crystal are dealt with by examining the set pairwise
*/
void
CrystalArray::RemoveIllegalOverlaps()
{
	mCalc->setupProgress("Removing illegal crystals", nil, nil, nil, -1, 0, GetNumXls(), 0, false);
	Crystal *thisXl, *otherXl;
	for (int i = 0; i <= GetNumXls() - 1; i++) {
		thisXl = (Crystal *) GetItemPtr(i);
		mCalc->progress(i);
		for (int j=i+1; j <= GetNumXls() - 1; j++) {
			otherXl = (Crystal *) GetItemPtr(j);
			float separation = thisXl->ctr.Distance(otherXl->ctr);
			if (separation < (thisXl->r + otherXl->r))	{	// if they overlap at all, then check for observability:
				Crystal *smaller, *larger;
				if (thisXl->r < otherXl->r) {
					smaller = thisXl;
					larger = otherXl;
				} else {
					smaller = otherXl;
					larger = thisXl;
				}
				if (separation + smaller->r <= larger->r) {	// then smaller is completely inside larger
					// this is a bit kludgy, but for ease of coding, I'm going to copy much of my FilterForObservability code
					Crystal newXl;
					newXl = *larger;
					RemoveItem(j, false);
					RemoveItem(i, false);
					PushXl(newXl, false);

					// force routine to redo this "i" loop by breaking out of j loop and
					// decrementing i so it will be incremented to the current value the
					// next time the for is executed.
					j = GetNumXls();
					i--;
				}
			}
		}
	}
	RebuildList();
}

#pragma mark ======== List Optimization Routines ========

// ---------------------------------------------------------------------------------
//		¥ RebuildLists
// ---------------------------------------------------------------------------------
void
CrystalArray::RebuildList()
{
	mList.clear();
	mList.reserve(GetNumXls());
	int numXls = GetNumXls();
	for (int i = 0; i <= numXls-1; i++) {
		Crystal *thisXl = &(array.at(i));

		// construct an XListElement
		XListElement temp;
		temp.index = i;	// If the size was 3, and we just added one, then the current size is 4, and the index
										// of the just-pushed Crystal is 3 (zero-based array)
		temp.location = thisXl->ctr.x;
		mList.push_back(temp);
//		std::vector<XListElement>::iterator it;
//		it=lower_bound (mList.begin(), mList.end(), temp);	// find the correct insert position for a sorted array
//		it = mList.insert (it, temp);	// insert sorted
	}
	std::sort(mList.begin(), mList.end(), XListElement::CompareListElement);
}

// ---------------------------------------------------------------------------------
//		¥ VerifyList
// ---------------------------------------------------------------------------------
bool
CrystalArray::VerifyList()
{
	float lastLoc = FLT_MIN;
	float thisLoc;
	Crystal *thisXl = nil;
	int numXls = mList.size();
	
	// Check to see if list is sorted, and that indices are correct
	for (int i=0; i <= numXls-1; i++) {
		thisLoc = (mList.at(i)).location;
		if (thisLoc < lastLoc) {
			return false;
		}
		lastLoc = thisLoc;
		thisXl = (Crystal *) GetItemPtr((mList.at(i)).index);
		if (thisXl->ctr.x != thisLoc) {
			return false;
		}
	}
	return true;
}

// ---------------------------------------------------------------------------------
//		¥ SortInternalList
// ---------------------------------------------------------------------------------
void
CrystalArray::SortInternalList()
{
	bool useSTLalgorithm = true;
	if (useSTLalgorithm) {
		std::vector<XListElement>::iterator start, end;
		start = mList.begin();
		end = mList.end();
		std::sort(start, end, XListElement::CompareListElement);	// hopefully, the STL algorithm is very fast, probably faster than
													// the version I coded.  That version is included below anyway.
	} else {
		// Pseudo-recursion version:  (Closely follows Numerical Recipes algorithm)
		short size = mList.size();
		short PSRMaxSize =  (2 * (log(size) / log(2)) * 1.2) + 4;	// extras for good measure, 2 for 2 elements per stack entry
		short	*PSRStack;
		bool done = false;
		while (!done) {
			try {
				#define QS_SWAP(a, b) temp = (a); (a)=(b); (b) = temp;
				#define MinSubarraySizeForQuickSort 5
				PSRStack = new short[PSRMaxSize+1];
				short PSRSize = 0;
				
				short curL = 0;	// "l" of NR text
				short curR = size-1;	// "ir" of NR text
				register short i, j, k;
				XListElement v;	// "a" of NR text
				XListElement temp;	// "temp" of NR text
				
				for (;;) {
					if (curR - curL < MinSubarraySizeForQuickSort) {	// Insertion sort when subarray small enough
						for (j = curL+1; j <= curR; j++) {
							v = mList.at(j);
							for (i = j-1; i >= curL; i--) {
								if (mList.at(i).location <= v.location) break;
								mList.at(i+1) = mList.at(i);
							}
							mList.at(i+1) = v;
						}
						if (PSRSize == 0) break;	// done!
						curR = PSRStack[PSRSize--];	// pop stack and start again
						curL = PSRStack[PSRSize--];
					} else {
						k = (curL + curR) >> 1;		// k is mean of curL & curR, since ">>" divides by two
													// Choose the median of left, center, and right elements as the pivot, value, v, and rearrange so that
													// mList.at(curL) <= mList.at(curL+1) <= mList.at(curR).
						QS_SWAP(mList.at(k), mList.at(curL+1));
						if (mList.at(curL).location > mList.at(curR).location) {
							QS_SWAP(mList.at(curL), mList.at(curR));
						}
						if (mList.at(curL+1).location > mList.at(curR).location) {
							QS_SWAP(mList.at(curL+1), mList.at(curR));
						}
						if (mList.at(curL).location > mList.at(curL+1).location) {
							QS_SWAP(mList.at(curL), mList.at(curL+1));
						}
						i = curL + 1;
						j = curR;
						v = mList.at(curL+1);
						
						// for the whole section, put everything less than the pivot (v) 
						// to the left, and greater than the pivot to the right
						for (;;) {
							do i++; while (mList.at(i).location < v.location) ;
							do j--; while (mList.at(j).location > v.location) ;
							if (j < i) break;
							QS_SWAP(mList.at(j), mList.at(i));
						}
						mList.at(curL+1) = mList.at(j);		// insert pivot value
						mList.at(j) = v;
						
						PSRSize += 2;
						
						// Push pointers to larger subarray on stack, process smaller subarray immediately
						if (PSRSize > PSRMaxSize)	throw PSRProblem();
						if (curR - i + 1 >= j-1) {
							// push the right side
							PSRStack[PSRSize] = curR;
							PSRStack[PSRSize-1] = i;
							curR = j-1;
						} else {
							// push the left side
							PSRStack[PSRSize] = j-1;
							PSRStack[PSRSize-1] = curL;
							curL = i;
						}
					}
				}
				done = true;
			} catch (PSRProblem) {
				delete[] PSRStack;
				PSRMaxSize *= 2;
			}
		}
		delete[] PSRStack;
	}
}

// ---------------------------------------------------------------------------------
//		¥ NearbyCount
// ---------------------------------------------------------------------------------
/* This method counts the number of crystals whose x-coordinate is within inDistance
	of inVal.  It also sets the lower and upper indices (within the sorted XList)
	to values that represent the limits of the desired range */
short
CrystalArray::NearbyCount(float inVal, float inDistance,
						  int &outLowerIndex, int &outUpperIndex)
{
#define MmListSizeForSearchAlgorithm 30
	
	float lowerValue = inVal - inDistance;
	float upperValue = inVal + inDistance;
	outLowerIndex = -1;
	outUpperIndex = -1;
	int numXls = mList.size();
	
	if (numXls < MmListSizeForSearchAlgorithm) {
		short inCount = 0;
		for (short i = 0; i <= numXls-1; i++) {
			if ((mList.at(i).location >= lowerValue) && (mList.at(i).location <= upperValue)) {
				inCount++;
				outUpperIndex = i;
				if (outLowerIndex == -1)
					outLowerIndex = i;
			} else {
				break;
			}
		}
		return inCount;
	} else {
		float	frxn;
		long	curLoc;
		short	direction;
		short lowerIndex, upperIndex;
		
		// find lower element to include the nearby list - this will be the largest element that is smaller
		// than inVal-inDistance
		frxn = dmh_min(1.0, lowerValue / ((mList.at(numXls-1)).location - (mList.at(0)).location) - (mList.at(0)).location);
		curLoc = dmh_max(0, (numXls * frxn) - 1);	// start at approx location
		direction = sign(lowerValue - (mList.at(curLoc)).location);
		float curXVal = (mList.at(curLoc)).location;
		while ((curLoc >= 0) && (curLoc <= numXls-1) && 
			   (lowerValue * direction > (mList.at(curLoc)).location * direction)) {
			curXVal = (mList.at(curLoc)).location;
			curLoc += direction;
		}
		curLoc = dmh_max(0, curLoc);	// make extra sure we didn't fall off the end someplace
		curXVal = (mList.at(curLoc)).location;

		// This next bit accounts for a possible series of values present in list that are equal to
		//	lowerValue, and also deals with the fact that we might be one index too high.  It's better
		//	include some extra crystals here rather than missing any that might matter.
		// We move down the list as long as we haven't fallen off the end, and we haven't gotten past
		// the target bottom value
		while ((curLoc > 0) && (lowerValue <= (mList.at(curLoc)).location)) {
			curXVal = (mList.at(curLoc)).location;
			curXVal = (mList.at(curLoc-1)).location;
			curLoc--;
		}

		curLoc = dmh_max(0, curLoc);	// make extra sure we didn't fall off the end someplace

		// curLoc is now the smallest value equal to or greater than lowerValue
		lowerIndex = curLoc;
		
		// find upper element to include in ioNearbyList
		frxn = dmh_min(1.0, upperValue / ((mList.at(numXls-1)).location - (mList.at(0)).location) - (mList.at(0)).location);
		curLoc = dmh_max(0, (numXls * frxn) - 1);	// start at approx location
		curXVal = (mList.at(curLoc)).location;
		direction = sign(upperValue - (mList.at(curLoc)).location);

		while ((curLoc >= 0) && (curLoc <= numXls-1) && 
			   (upperValue * direction > (mList.at(curLoc)).location * direction)) {
			curXVal = (mList.at(curLoc)).location;
			curLoc += direction;
		}
		curLoc = dmh_min(numXls - 1, curLoc);	// make extra sure we didn't fall off the end someplace
		curXVal = (mList.at(curLoc)).location;
		
		// need to account for series of values present in list that are equal to upperValue
		while ((curLoc <= numXls-1) && (upperValue >= (mList.at(curLoc)).location)) {
			curXVal = (mList.at(curLoc)).location;
			curXVal = (mList.at(curLoc-1)).location;
			curLoc++;
		}
		
		curLoc = dmh_min(numXls - 1, curLoc);	// make extra sure we didn't fall off the end someplace

		// curLoc is now the largest value equal to or less than upperValue
		upperIndex = curLoc;
		
		outLowerIndex = lowerIndex;
		outUpperIndex = upperIndex;
		return (upperIndex - lowerIndex + 1);
	}
	
	// should never get here
	return -1;
}

// ---------------------------------------------------------------------------------
//		¥ NearbyXls
//	This function is the primary purpose of having the List.  We can get a quick list
//	of crystals near a given crystal.
// ---------------------------------------------------------------------------------
std::vector<int>
CrystalArray::NearbyXls(Point3DFloat inPt, float inDistance)
{
	int lower, upper;
	short listSize = NearbyCount(inPt.x, inDistance, lower, upper);
	std::vector<int> firstList;
	std::vector<int> newList;
	if (listSize <= 0) {
		newList.push_back(0);
	} else {
		for (long curLoc = lower; curLoc <= upper; curLoc++) {
			firstList.push_back((mList.at(curLoc)).index);
		}
		int firstListSize = firstList.size();
		
		// look through list, and see if each crystal is in range
		newList.reserve(firstListSize);
		Crystal *thisXl;
		for (int i = 0; i <= firstListSize-1; i++) {
			int index = firstList.at(i);
			thisXl = &(array.at(index));
			thisXl = &(array.at((firstList.at(i))));
			if ((::fabs(thisXl->ctr.y - inPt.y) < inDistance) &&
				(::fabs(thisXl->ctr.z - inPt.z) < inDistance)) {
				newList.push_back(firstList.at(i));
			}
		}		
	}
	return newList;
}


/* NOT CALLED
// ---------------------------------------------------------------------------------
//		¥ AddToList
// ---------------------------------------------------------------------------------
void
CrystalArray::AddToList(XListElement *mList, float inVal, long inIndex, long *ioListSizeAlloc)
{
	if (numXls == 0) {
		(mList.at(numXls)).index = inIndex;
		(mList.at(numXls)).location = inVal;
	} else {
		if (numXls+1 >= *ioListSizeAlloc) {
			// need to re-allocate, larger
			*ioListSizeAlloc = (float) *ioListSizeAlloc * 1.1;					// increase by 10%
			mList = ResizeList(mList, *ioListSizeAlloc);
		}
		// find insert location
		float	frxn = min(1.0, inVal / ((mList.at(numXls-1)).location - (mList.at(0)).location) - (mList.at(0)).location);
		long	curLoc = max(0, (numXls * frxn) - 1);	// start at approx location
		short	direction = sign(inVal - (mList.at(curLoc)).location);
		while ((curLoc >= 0) && (curLoc <= numXls-1) && 
			   (inVal * direction > (mList.at(curLoc)).location * direction)) {
			curLoc += direction;
		}
		curLoc += -((direction - 1) / 2);	// does nothing if direction was +1, adds 1 if direction was -1
		if (numXls > curLoc)
			::BlockMove(mList + ((curLoc) * sizeof(XListElement)),
						mList + ((curLoc+1) * sizeof(XListElement)),
						sizeof(XListElement) * (numXls - curLoc));	// shift to make room
		(mList.at(curLoc)).index = inIndex;
		(mList.at(curLoc)).location = inVal;
	}
}

// ---------------------------------------------------------------------------------
//		¥ RemoveFromList
// ---------------------------------------------------------------------------------
bool
CrystalArray::RemoveFromList(XListElement *mList, float inVal, long inIndex)
{
	if (numXls > 0) {
		// find location
		float	frxn = min(1.0, inVal / ((mList.at(numXls-1)).location - (mList.at(0)).location) - (mList.at(0)).location);
		long	curLoc = (numXls-1) * frxn;	// start at approx location
		short	direction = sign(inVal - (mList.at(curLoc)).location);
		while ((curLoc >= 0) && (curLoc <= numXls-1) && 
			   (inVal * direction - (mList.at(curLoc)).location * direction > fabs(inVal/100))) {
			curLoc += direction;
		}
		while (inVal - (mList.at(curLoc)).location <= fabs(inVal/100)) {
			if (inIndex == (mList.at(curLoc)).index) {
				::BlockMove(mList + ((curLoc+1) * sizeof(XListElement)),
							mList + ((curLoc) * sizeof(XListElement)),
							sizeof(XListElement) * max(0, (numXls - curLoc - 1)));	// shift to make room
				return true;
			} else {
				curLoc += direction;
			}
		}
		
		SignalPStr_("Tried to remove from a list, but item wasn't there!");
		return false;
	}
	return true;	// shouldn't get here
}
*/


