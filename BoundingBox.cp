// ===========================================================================
//	BoundingBox.cp
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================

#include "BoundingBox.h"
#include "MathStuff.h"
#import "Calculator.h"
#import "HoleSet.h"
#import "stringFile.h"

// ---------------------------------------------------------------------------
//		¥ BoundingBox
// ---------------------------------------------------------------------------
//	Default Constructor
BoundingBox::BoundingBox(Calculator *inCalc)
	: SideSet()
{
	mCalc = inCalc;
	mPrefs = mCalc->getPrefs();

	mPtArray = nil;
	theXls = nil;
}

// ---------------------------------------------------------------------------
//		¥ BoundingBox (stringFile*)
// ---------------------------------------------------------------------------
//	Standard Constructor
BoundingBox::BoundingBox(Calculator *inCalc, stringFile *inFile)
	: SideSet()
{
    try {
        mCalc = inCalc;
        mPtArray = nil;
        theXls = new CrystalArray(inCalc);
        theXls->SetFile(inFile);
        theXls->ReadMergeFile();
        mPrefs = mCalc->getPrefs();

        // Temporarily set the box type from the Merge file header (stored in the CrystalArray)
        //	If bounds were set in the input file, that should override any sample shape setting
        //	by the user.
        switch (theXls->GetBounds()) {
            case kBoundsNone:
                mType = kSidesBox;
                break;
            case kBoundsRP:
                mType = kRPBox;
                mPrefs->sampleShape = kRectPrism;
                break;
            case kBoundsCyl:
                mType = kCylBox;
                mPrefs->sampleShape = kCylinder;
                break;
        }
        
        if (((theXls->GetBounds() != kBoundsNone) && CheckedAgainstBounds()) 
            && !(mPrefs->discardNegs && theXls->GetNegRadii())) {
            /* If:	1. The file had bounds stated
             2. CheckedAgainstBounds suggested we discard crystals outside the bounds (returned true)
             3. It is not the case that there are negative-radii crystals to be discarded
             Then: we should discard the crystals outside the bounds.  (Criterion 3 is important,
             because negative radii crystals will be near the edges.  If they are discarded, then the 
             bounds no longer really apply.)
             Note: by the time execution arrives here, the bounding box must have already been set as
             a bounds-bearing type (not a kSidesBox).*/
            Crystal *thisXl;
            for (int i=0; i <= theXls->GetNumXls() - 1; i++) {
                thisXl = theXls->GetItemPtr(i);
                if (!PointInBox(thisXl->ctr)) {
                    theXls->RemoveItem(i, false);
                    i--;
                }
            }
            theXls->RebuildList();
            BetterInscribedBox();	// shouldn't need this; but couldn't
        }
    } catch (...) {
        std::string logStr ("In BoundingBox::BoundingBox .  Catching error and re-throwing.\n");
        mCalc->log(logStr);
        throw kUserCanceledErr;
    }


	// This is now handled in Calculator::reduceOneDataset

	/*	if (mPrefs->applyObservabilityFilter) {
		theXls->RemoveIllegalOverlaps();
	}
*/
}

// ---------------------------------------------------------------------------
//		¥ BoundingBox (BoundingBox&)
// ---------------------------------------------------------------------------
//	Copy Constructor
BoundingBox::BoundingBox(BoundingBox &inBBox)
	: SideSet(inBBox)
{
	if (inBBox.mPtArray) {
		mPtArray = new NumPtArray((const NumPtArray &)(*inBBox.mPtArray));
	} else {
		mPtArray = nil;
	}
	xMin = inBBox.xMin;
	xMax = inBBox.xMax;
	yMin = inBBox.yMin;
	yMax = inBBox.yMax;
	zMin = inBBox.zMin;
	zMax = inBBox.zMax;
	mCalc = inBBox.mCalc;
	mPrefs = inBBox.mPrefs;
	if (inBBox.theXls) {
		theXls = new CrystalArray(mCalc);
		theXls->Copy(inBBox.theXls);
	} else {
		theXls = nil;
	}
}

// ---------------------------------------------------------------------------
//		¥ BoundingBox
// ---------------------------------------------------------------------------
void
BoundingBox::Copy(BoundingBox &inBBox)
{
	SideSet::Copy(inBBox);
	if (mPtArray) delete mPtArray;
	if (inBBox.mPtArray) {
		mPtArray = new NumPtArray((const NumPtArray &)(*inBBox.mPtArray));
	} else {
		mPtArray = nil;
	}
	mCalc = inBBox.mCalc;
	if (theXls != nil) {
		delete theXls;
		theXls = nil;
	}
	if (inBBox.theXls) {
		theXls = new CrystalArray(mCalc);
		theXls->Copy(inBBox.theXls);
	} else {
		theXls = nil;
	}
}

// ---------------------------------------------------------------------------
//		¥ ~BoundingBox
// ---------------------------------------------------------------------------
//	Default Destructor

BoundingBox::~BoundingBox()
{
//	Unlock();
	if (mPtArray != nil) {
		delete mPtArray;
		mPtArray = nil;
	}
	if (theXls != nil) {
		delete theXls;
		theXls = nil;
	}
}

// ---------------------------------------------------------------------------
//		¥ saveShavedIntegrateFile
// ---------------------------------------------------------------------------
void		
BoundingBox::saveShavedIntegrateFile(short inShaveIteration)
{
	// Make a new filename for the shaved file.  If the input file was foo.Int,
	// then the shave files should look like: foo_shave_1.Int
	
	// The input file could end with .txt, .int, or .Int, or possibly none of these
	
	CFStringRef inFileCFPath = (theXls->GetFile())->getPath();
	char inFileCString[kStdStringSize];
	bool result = CFStringGetCString (inFileCFPath, inFileCString, (CFIndex) kStdStringSize, kCFStringEncodingUTF8);
	std::string inFilePath (inFileCString);
	int lastPeriod = inFilePath.find('.');
	std::string shaveFilePath = inFilePath.erase(lastPeriod);	// erase everything after the last period
	shaveFilePath += "_shave_";
	char numStr[8];
	sprintf(numStr, "%d.Int", inShaveIteration);
	shaveFilePath += numStr;
	
	stringFile shaveFile(true, shaveFilePath.c_str());
	
	int numXls = theXls->GetNumXls();
	mCalc->setupProgress("Saving shaved dataset as Integrate file", nil, nil, nil, -1, 0, numXls-1, 0, false);
	
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFDictionaryRef bundleInfoDict = CFBundleGetInfoDictionary( mainBundle );
    CFStringRef mVersionString;
    // If we succeeded, look for our property.
    if ( bundleInfoDict != NULL ) {
        mVersionString = (CFStringRef) CFDictionaryGetValue( bundleInfoDict, CFSTR("CFBundleVersion") );
    }
	
	std::string thisLine = "Reduce3D version ";
	char tempLine[kStdStringSize];
	result = CFStringGetCString (mVersionString, tempLine, (CFIndex) kStdStringSize, kCFStringEncodingUTF8);
	thisLine += tempLine;
	thisLine += " || ";
	// write identifier for file type
	if (mPrefs->doShave) {
		sprintf(tempLine, "Shaved %.2f%%,", inShaveIteration * mPrefs->shaveIncrement);
		thisLine += tempLine;
		if (mPrefs->keepAspectRatios) {
			thisLine += "keeping AR,";
		} else switch (mPrefs->shaveXYZ) {
			case kX: thisLine += "in X, "; break;
			case kY: thisLine += "in Y, "; break;
			case kZ: thisLine += "in Z, "; break;
		}
			switch (mPrefs->direction) {
				case kFromMax: thisLine += "from lg.values"; break;
				case kSymmetric: thisLine += "from outside"; break;
				case kFromMin: thisLine += "from sm.values"; break;
			}
	}
	std::string intComment = theXls->GetIntComment();
	writeIntegrateFile(&shaveFile, thisLine.c_str(), intComment.c_str());
}

// ---------------------------------------------------------------------------
//		¥ writeIntegrateFile
// ---------------------------------------------------------------------------
void		
BoundingBox::writeIntegrateFile(stringFile *saveFile, const char *firstLine, const char *commentLine) 
{
	int numXls = theXls->GetNumXls();
	saveFile->putOneLine(firstLine);
	
	// write incoming comment line
	saveFile->putOneLine(commentLine);

	char tempLine[kStdStringSize];
	// write NumXls line
	sprintf(tempLine, "Number of crystals:\t%d", numXls);
	saveFile->putOneLine(tempLine);
	
	// write Volume line
	sprintf(tempLine, "Total volume:\t%.5f", Volume());
	saveFile->putOneLine(tempLine);
	
	// write Bounds line
	if ((mType == kRPBox) || (mType == kCubeBox)) {
		Point3DFloat lower = theXls->LowerBound();
		Point3DFloat upper = theXls->UpperBound();
		sprintf(tempLine, "Bounds:\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f",
				lower.x, lower.y, lower.z, upper.x, upper.y, upper.z);
		saveFile->putOneLine(tempLine);
	} else if (mType == kCylBox) {
		Point3DFloat ctr = theXls->GetCtr();
		sprintf(tempLine, "Ctr/R/H:\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f",
				ctr.x, ctr.y, ctr.z, theXls->GetRadius(), theXls->GetHeight());
		saveFile->putOneLine(tempLine);
	}
	// write Xl data
	Crystal *thisXl;
	for (int i = 0; i <= numXls-1; i++) {
		mCalc->progress(i);
		thisXl = theXls->GetItemPtr(i);
		sprintf(tempLine, "%d\t%.5f\t%.5f\t%.5f\t%.5f\t%d\t%ld",
				i+1, thisXl->ctr.x, thisXl->ctr.y, thisXl->ctr.z, thisXl->r, thisXl->ctrSlice, thisXl->ctrID);
		saveFile->putOneLine(tempLine);
	}
}


#pragma mark Setting up and prep for convex hull
// ---------------------------------------------------------------------------
//		¥ CheckedAgainstBounds
// ---------------------------------------------------------------------------
/* This method checks the set of crystals against the bounds stated in the input
file (if there were any).  It warns the user if any crystals are outside the
bounds, and optionally discards the bad ones. 
	Notably, this method also adopts into the current Bounding Box the bounds 
read by the CrystalArray from the input file header, if any*/
bool
BoundingBox::CheckedAgainstBounds()
{
	Crystal *thisXl;
	long numOutside = 0;
	Point3DFloat Lower, Upper;
	if (theXls->GetBounds() == kBoundsRP) {
		Lower = theXls->LowerBound();
		Upper = theXls->UpperBound();
		mCtr = (Lower + Upper) / 2.0;
		mType = kRPBox;
		mSideLenX = Upper.x - Lower.x;
		mSideLenY = Upper.y - Lower.y;
		mSideLenZ = Upper.z - Lower.z;
		xMax = yMax = zMax = Upper;
		xMin = yMin = zMin = Lower;
	} else if (theXls->GetBounds() == kBoundsCyl) {
		mCtr = theXls->GetCtr();
		mRadius = theXls->GetRadius();
		mHeight = theXls->GetHeight();
		xMax = yMax = zMax = xMin = yMin = zMin = mCtr;
		xMax.x = mCtr.x + mRadius;
		yMax.y = mCtr.y + mRadius;
		zMax.z = mCtr.z + 0.5 * mHeight;
		xMin.x = mCtr.x - mRadius;
		yMin.y = mCtr.y - mRadius;
		zMin.z = mCtr.z - 0.5 * mHeight;
		mType = kCylBox;
	} else {
		throw("Problem in CheckedAgainstBounds.");
	}
	
	int numXls = theXls->GetNumXls();
	for (int i=0; i <= numXls - 1; i++) {
		thisXl = (Crystal *) theXls->GetItemPtr(i);
		if (!PointInBox(thisXl->ctr)) {
			numOutside++;
		}
	}
	if (numOutside > 0) {
		char msg[kStdStringSize];
		sprintf(msg, "There is a problem: %ld crystals (out of %d) are outside the bounds stated in the input file.  Do you want to discard the offending crystals, or fit a new bounding box?", numOutside, numXls);
		short result = mCalc->postError(msg, "Bounds Error", "Discard Crystals|New Bounding Box", 0, -1);

		if (result == 0) {
			// User chose Discard Crystals, which means that the Bounds are good and we want to adapt the crystals to fit them
			if (mPrefs->verbose) {
				std::string logStr ("In CheckedAgainstBounds.  There were crystals outside the stated bounds, and the user chose to discard them.\n");
				mCalc->log(logStr);
			}
			return true;
		} else {
			// User chose to make a new bounding box around the data set.  This mwans that the bounds are bad.
			if (mPrefs->verbose) {
				std::string logStr ("In CheckedAgainstBounds.  There were crystals outside the stated bounds, and the user chose to make a new bounding box.\n");
				mCalc->log(logStr);
			}
			return false;
		}
	} else {
		return true;
	}
}

// ---------------------------------------------------------------------------
//		¥ FindBoundPoints
// ---------------------------------------------------------------------------
/* Finds the points (i.e., crystal centers) with the maximum and minimum 
values in each dimension. */
void
BoundingBox::FindBoundPoints()
{
	NumberedPt thisPt;
	short i;
	
	thisPt = (*mPtArray)[0];
	xMin = xMax = yMin = yMax = zMin = zMax = thisPt;
	for (i  = 1 ; i  <= mPtArray->GetCount() - 1 ; i ++) {
		thisPt = (*mPtArray)[i];
		if (thisPt.x < xMin.x )
			xMin = thisPt;
		if (thisPt.x > xMax.x )
			xMax = thisPt;
		if (thisPt.y < yMin.y )
			yMin = thisPt;
		if (thisPt.y > yMax.y )
			yMax = thisPt;
		if (thisPt.z < zMin.z )
			zMin = thisPt;
		if (thisPt.z > zMax.z )
			zMax = thisPt;
	}
	if (mPrefs->verbose) {
		mCalc->log("Looked for bounding crystal centers (BoundingBox::FindBoundPoints).  Found these:");
		char logMsg[kStdStringSize];
		sprintf (logMsg, "\t xMin = (%f, %f, %f)\n", xMin.x, xMin.y, xMin.z);
		mCalc->log(logMsg);
		sprintf (logMsg, "\t yMin = (%f, %f, %f)\n", yMin.x, yMin.y, yMin.z);
		mCalc->log(logMsg);
		sprintf (logMsg, "\t zMin = (%f, %f, %f)\n", zMin.x, zMin.y, zMin.z);
		mCalc->log(logMsg);
		sprintf (logMsg, "\t xMax = (%f, %f, %f)\n", xMax.x, xMax.y, xMax.z);
		mCalc->log(logMsg);
		sprintf (logMsg, "\t yMax = (%f, %f, %f)\n", yMax.x, yMax.y, yMax.z);
		mCalc->log(logMsg);
		sprintf (logMsg, "\t zMax = (%f, %f, %f)\n", zMax.x, zMax.y, zMax.z);
		mCalc->log(logMsg);
	}	
}

// ---------------------------------------------------------------------------
//		¥ FindCenter
// ---------------------------------------------------------------------------
/* Given the maximum and minimum values in the x,y, and z dimension, finds a
point between them all by averaging. This gives a point which is guaranteed
to be somewhere inside the solid. */
void
BoundingBox::FindCenter()
{
	mCtr.x = (xMax.x + xMin.x) / 2.0;
	mCtr.y = (yMax.y + yMin.y) / 2.0;
	mCtr.z = (zMax.z + zMin.z) / 2.0;
	if (mPrefs->verbose) {
		char logMsg[kStdStringSize];
		sprintf (logMsg, "Found center from max/min points: (%f, %f, %f)\n", mCtr.x, mCtr.y, mCtr.z);
		mCalc->log(logMsg);
	}
}

// ---------------------------------------------------------------------------
//		¥ FindBetterCenter
// ---------------------------------------------------------------------------
/* Finds the average of the coordinates.  If this is inside the BBox (and it
must be), then this point is taken as the new center. */
void
BoundingBox::FindBetterCenter()
{
	Point3DFloat thePt;
	thePt = theXls->Average();
	if (PointInBox(thePt)) {
		mCtr = thePt;
	} else {
		throw("There's some problem with the data set.  BoundingBox::FindBetterCenter couldn't.");
	}
}

// ---------------------------------------------------------------------------
//		¥ PrepForHullOrPrimitive
// ---------------------------------------------------------------------------
void
BoundingBox::PrepForHullOrPrimitive()
{
	mPtArray = new NumPtArray(theXls);	// this is basically a secondary array with each point containing the index and center of a crystal
	FindBoundPoints();
	FindCenter();
}

// ---------------------------------------------------------------------------
//		¥ DiscardInteriorPoints
// ---------------------------------------------------------------------------
/* This routine uses the max and min points of each dimension to construct a
box, the inscribed octahedron formed by the mins and maxes in each dimension.
All of the points in the set are examined to see if they fall within this box
(and not on the edge).  Those that do are discarded, since they automatically
won't be on the boundary.  Ideally, this should weed out a considerable number
of points so they won't slow the difficult calculation of the convex hull itself
later.  This inscribed box is superceded by the one made by BetterInscribedBox.  */
void
BoundingBox::DiscardInteriorPoints()
{
	// If any max or min points are identical, then the inscribed box cannot be made
	if ((xMax == yMax || xMax == yMin || xMax == zMax || xMax == zMin) ||
		(xMin == yMax || xMin == yMin || xMin == zMax || xMin == zMin) ||
		(yMax == xMax || yMax == xMin || yMax == zMax || yMax == zMin) ||
		(yMin == xMax || yMin == xMin || yMin == zMax || yMin == zMin) ||
		(zMax == xMax || zMax == xMin || zMax == yMax || zMax == yMin) ||
		(zMin == xMax || zMin == xMin || zMin == yMax || zMin == yMin)) {
		mInscribedBox = nil;
	} else {
		mInscribedBox = new SideSet;	// default is kSidesBox
		mInscribedBox->SetCtr(mCtr);
		Side thisSide;
		thisSide.pt1 = zMax;
		thisSide.pt2 = xMax;
		thisSide.pt3 = yMin;
		mInscribedBox->AlignSide(thisSide);
		mInscribedBox->PushSide(thisSide);
		thisSide.pt1 = zMax;
		thisSide.pt2 = yMin;
		thisSide.pt3 = xMin;
		mInscribedBox->AlignSide(thisSide);
		mInscribedBox->PushSide(thisSide);
		thisSide.pt1 = zMax;
		thisSide.pt2 = xMin;
		thisSide.pt3 = yMax;
		mInscribedBox->AlignSide(thisSide);
		mInscribedBox->PushSide(thisSide);
		thisSide.pt1 = zMax;
		thisSide.pt2 = yMax;
		thisSide.pt3 = xMax;
		mInscribedBox->AlignSide(thisSide);
		mInscribedBox->PushSide(thisSide);
		thisSide.pt1 = zMin;
		thisSide.pt2 = xMax;
		thisSide.pt3 = yMax;
		mInscribedBox->AlignSide(thisSide);
		mInscribedBox->PushSide(thisSide);
		thisSide.pt1 = zMin;
		thisSide.pt2 = yMax;
		thisSide.pt3 = xMin;
		mInscribedBox->AlignSide(thisSide);
		mInscribedBox->PushSide(thisSide);
		thisSide.pt1 = zMin;
		thisSide.pt2 = xMin;
		thisSide.pt3 = yMin;
		mInscribedBox->AlignSide(thisSide);
		mInscribedBox->PushSide(thisSide);
		thisSide.pt1 = zMin;
		thisSide.pt2 = yMin;
		thisSide.pt3 = xMax;
		mInscribedBox->AlignSide(thisSide);
		mInscribedBox->PushSide(thisSide);
		mInscribedBox->MakeAllInVects();
		
		short i = 0;
		short numRemoved = 0;
		
		NumberedPt curPt;
		mCalc->setupProgress("Optimizing data set for convex hull operation...", nil, nil, "Finding convex hull", -1, 0, 100, 0, true);
		
		while (i < mPtArray->GetCount() - 1)	{
			mCalc->progress((i * 100) / mPtArray->GetCount());
			if (mCalc->shouldStopCalculating()) throw(kUserCanceledErr);

			curPt = (*mPtArray)[i];
			if (mInscribedBox->RawPointInBox(curPt)) {
				// if this point is in the inscribed octahedron, then remove it from the array
				mPtArray->RemoveItem(i);
				numRemoved++;
			} else
				i++;
		}
		if (mPrefs->verbose) {
			char logMsg[kStdStringSize];
			sprintf(logMsg, "In BoundingBox::DiscardInteriorPoints.  Discarded a total of %d points.", numRemoved);
			mCalc->log(logMsg);
		}
		Clear();
	}
}


#pragma mark Finding the convex hull
// ---------------------------------------------------------------------------
//		¥ FindConvexHull
// ---------------------------------------------------------------------------
void
BoundingBox::FindConvexHull()
{
	short	sideNum=0, edgeNum=0;
	std::string theStr;
	
	if (mPrefs->verbose) mCalc->log("Finding Convex Hull.\n");
		
	theXls->SetBounds(kBoundsNone);	// remove bounds from crystal array
	mType = kSidesBox;	// we are going to be a Sides box (for now at least; if the user selected a different
						// primitive, then that primitive will be adopted later
	PrepForHullOrPrimitive();
	
	if (false) {
		// use old algorithm
		DiscardInteriorPoints();

		mCalc->setupProgress("Finding Sides in convex hull...", nil, nil, nil, -1, 0, 0, 0, true);
		char progMsg[200];
		short numSides;
		FindFirstSide();
		while (FindNextEdge(sideNum, edgeNum)) {
			FoldEdge(sideNum, edgeNum);
			numSides = GetCount();
			if ((numSides >= 10) && (numSides % 10 == 0)) {
				sprintf(progMsg, "Finding Sides in convex hull... Found %d sides.", numSides);
				if (mCalc->shouldStopCalculating()) throw(kUserCanceledErr);
				mCalc->setProgMessage(progMsg);
			}
		}
		mCalc->setProgMessage("Finding Sides in convex hull... Found all sides.");
	} else {

/*			//testing Angle()
		Point3DFloat vect1(0,1,0);
		Point3DFloat vect2(1,1,0);
		double testAngle=vect1.Angle(vect2);
		for (double i=0.11; i <=2.0; i+=0.1) {
			Point3DFloat vect3(1, 1-i, 0);
			testAngle=vect1.Angle(vect3);
			double dotProd = (vect1 * vect3);
			if ((dotProd < 0) && (testAngle > M_PI_2)) {
				bool somethingFishy=true;
				throw;
			}
		}
*/			
		char progMsg[200];
		mCalc->setupProgress("Finding Sides in convex hull...", nil, nil, nil, -1, 0, 100, 0, false);
		FindInitialTetrahedron();	// make the first tetrahedron and discard all interior points
		double lastVolume = Volume();
//			ExportBoxData();

		while (UnfacetedPointsExist()) {
			FindNextVertexAndMakeFacets();
			
			double thisVolume = Volume();
			if (thisVolume < lastVolume) {
				ExportBoxData();
				throw("Problem here");
			}
//				ExportBoxData();
			lastVolume = thisVolume;
			
			sprintf(progMsg, "Finding Sides in convex hull... Found %d sides.", GetCount());
			if (mCalc->shouldStopCalculating()) throw(kUserCanceledErr);
			mCalc->setProgMessage(progMsg);
			double fractionDone = 1.0 - ((double)mPtArray->GetCount() / (double)theXls->GetNumXls());
			mCalc->progress(::trunc(100 * fractionDone));
		}
		mCalc->setProgMessage("Finding Sides in convex hull... Found all sides.");
		if (mPrefs->outputHull) {
			ExportBoxData();
		}
	}

	MakeAllInVects();

	// shouldn't need this, but there's some problem occasionally:
	Side	*curSide;
	short numSides = GetCount();
	for (short i = 0; i <= numSides - 1; i++) {
		curSide = GetItemPtr(i);
		AlignSide(*curSide);
		UpdateSide(i, *curSide);
	}
	FindBetterCenter();
	BetterInscribedBox();	// Now that we have a good convex hull, we want to optimize
							// future Bounding-Box-related operations by creating a new inscribed box
							// and an exscribed box, too (despite the name)
}
// ---------------------------------------------------------------------------
//		¥ FindNextVertexAndMakeFacets
// ---------------------------------------------------------------------------
void
BoundingBox::FindNextVertexAndMakeFacets()
{
	Point3DFloat tempCtr = CalcCtr();
	this->SetCtr(tempCtr);	// need to set this to make the Volume funciton work correctly
	char logMsg[255];
	sprintf (logMsg, "\t Found temporary center: (%f, %f, %f)\n", tempCtr.x, tempCtr.y, tempCtr.z);
	mCalc->log(logMsg);

	short pointNum = 0;
	while (((*mPtArray)[pointNum]).flag == true) {
		pointNum++;
	}	
	// flag the point in the original array, and move it to the end.  It will either remain flagged, if 
	// the point happens to be a true convex hull point, or will get deleted by being found inside the
	// hull at some future point.
	NumberedPt newVertex = (*mPtArray)[pointNum];
	newVertex.flag = true;
	mPtArray->PushPt(newVertex);
	mPtArray->RemoveItem(pointNum);
	
	// figure out what facets this point can "see"
	SideSet visibleSides;
	Side *thisSide;

//    !!!	something's wrong here - the set of sides can somehow dwindle to none!
	
	for (int i = 0 ; i <= array.size() - 1 ; i ++) {
		thisSide = &(array.at(i));
		if (thisSide->IsWithout(newVertex)) {
			// the vertex can see this side, so add it to the list of seen sides
			visibleSides.PushSide(*thisSide);
			
			// remove it from the current set of sides, since it will be superceded by the new facets
			RemoveSide(i);
			i--;	// decrement to be sure we don't skip any
		}
	}
	
	// Go through the set of visible sides and find the edges of that set (the "horizon")
	short numVisibleSides = visibleSides.GetCount();
	// first mark all edges as unshared (false)
	for (short j=0; j < numVisibleSides; j++) {
		Side *thisSide = visibleSides.GetItemPtr(j);
		thisSide->edge1 = false;
		thisSide->edge2 = false;
		thisSide->edge3 = false;
	}
	
	short numSharedPairs = 0;
	// examine the Sides pairwise and look for shared edges
	for (short j=0; j < numVisibleSides; j++) {
		Side *thisSide = visibleSides.GetItemPtr(j);
		for (short k=j+1; k < numVisibleSides; k++) {
			Side *otherSide = visibleSides.GetItemPtr(k);
			bool foundShared = false;
			// see if the two sides share any edges in common, and mark the shared edges true
			if (thisSide->PointOnSide(otherSide->pt1) && thisSide->PointOnSide(otherSide->pt2)) {
				// we know that edge3 (from pt1-pt2) of the other side is the one that is shared, so mark that:
				otherSide->edge3 = true;
				foundShared = true;
			} else if (thisSide->PointOnSide(otherSide->pt2) && thisSide->PointOnSide(otherSide->pt3)) {
				// we know that edge1 (from pt2-pt3) of the other side is the one that is shared, so mark that:
				otherSide->edge1 = true;
				foundShared = true;
			} else if (thisSide->PointOnSide(otherSide->pt1) && thisSide->PointOnSide(otherSide->pt3)) {
				// we know that edge2 (from pt1-pt3) of the other side is the one that is shared, so mark that:
				otherSide->edge2 = true;
				foundShared = true;
			}
			
			if (foundShared) {
				// we found a shared edge, and marked the right one for the otherSide->
				// we can look for the unshared point on thisSide->  The other two must be shared.
				if (!otherSide->PointOnSide(thisSide->pt1)) {
					thisSide->edge1 = true;
				} else if (!otherSide->PointOnSide(thisSide->pt2)) {
					thisSide->edge2 = true;
				} else if (!otherSide->PointOnSide(thisSide->pt3)) {
					thisSide->edge3 = true;
				}
				numSharedPairs++;
			}				
		}
	}
	
	// Now we have marked all the shared edges as true.  The false edges are the horizon line (unshared
	// edges).  These will be used to create new facets (Sides)
	// examine the Sides pairwise and look for shared edges
	Side newSide;
	for (short j=0; j < numVisibleSides; j++) {
		Side *thisSide = visibleSides.GetItemPtr(j);
		if (!thisSide->edge1) {	// edge1 is a horizon edge
			newSide.pt1=newVertex;
			newSide.pt2=thisSide->pt2;
			newSide.pt3=thisSide->pt3;
			newSide.MakeInVect(thisSide->pt1);	// we know that thisSide's pt1 is interior of the new facet
			PushSide(newSide);
			char logMsg[255];
			sprintf (logMsg, "\t Made a new side: (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)\n", 
					 newSide.pt1.x, newSide.pt1.y, newSide.pt1.z,
					 newSide.pt2.x, newSide.pt2.y, newSide.pt2.z,
					 newSide.pt3.x, newSide.pt3.y, newSide.pt3.z
					 );
			mCalc->log(logMsg);
			
		}
		if (!thisSide->edge2) {	// edge2 is a horizon edge
			newSide.pt1=newVertex;
			newSide.pt2=thisSide->pt1;
			newSide.pt3=thisSide->pt3;
			newSide.MakeInVect(thisSide->pt2);	// we know that thisSide's pt2 is interior of the new facet
			PushSide(newSide);
			char logMsg[255];
			sprintf (logMsg, "\t Made a new side: (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)\n", 
					 newSide.pt1.x, newSide.pt1.y, newSide.pt1.z,
					 newSide.pt2.x, newSide.pt2.y, newSide.pt2.z,
					 newSide.pt3.x, newSide.pt3.y, newSide.pt3.z
					 );
			mCalc->log(logMsg);
		}
		if (!thisSide->edge3) {	// edge3 is a horizon edge
			newSide.pt1=newVertex;
			newSide.pt2=thisSide->pt1;
			newSide.pt3=thisSide->pt2;
			newSide.MakeInVect(thisSide->pt3);	// we know that thisSide's pt3 is interior of the new facet
			PushSide(newSide);
			char logMsg[255];
			sprintf (logMsg, "\t Made a new side: (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)\n", 
					 newSide.pt1.x, newSide.pt1.y, newSide.pt1.z,
					 newSide.pt2.x, newSide.pt2.y, newSide.pt2.z,
					 newSide.pt3.x, newSide.pt3.y, newSide.pt3.z
					 );
			mCalc->log(logMsg);
		}
	}

	// Discard the interior points
	short discarded = 0;
	for (short k=0; k < mPtArray->GetCount(); k++) {
		if (PointInBox((*mPtArray)[k], -0.00001) && !PointOnBox((*mPtArray)[k])) {	// the tolerance value for PointInBox is an extra angular measure in radians.  Negative values will cause close points to register as being inside the box.
			mPtArray->RemoveItem(k);
			k--;
			discarded++;
		}
	}
//	char logMsg[255];
	sprintf (logMsg, "\t Hull now has %d sides and %f volume. Found %d shared edge pairs. Discarded %d interior points.  %d points remain\n", GetCount(), Volume(), numSharedPairs,  discarded, mPtArray->GetCount());
	mCalc->log(logMsg);
}

// ---------------------------------------------------------------------------
//		¥ UnfacetedPointsExist
// ---------------------------------------------------------------------------
bool
BoundingBox::UnfacetedPointsExist()
{
	bool returnValue=false;
	for (short k=0; k < mPtArray->GetCount(); k++) {
		if (((*mPtArray)[k]).flag == false) {
			returnValue = true;
			return returnValue;
		}
	}	
	return returnValue;
}

// ---------------------------------------------------------------------------
//		¥ FindInitialTetrahedron
// ---------------------------------------------------------------------------
/*  This chooses four random crystals and makes a tetrahedron out of them, which is the 
	first step in creating the convex hull (the new way).
 */
void 
BoundingBox::FindInitialTetrahedron()
{
	mPtArray->Randomize();
	NumPtArray tetPts;
	Point3DFloat average;
	// Choose four random points in the set
	NumberedPt tempPt;
	for (short i=0; i<4; i++) {
		tempPt= (*mPtArray)[0];
		tempPt.flag = true;

		tetPts.PushPt(tempPt);
		
		// flag the point in the original array
		mPtArray->RemoveItem(0);
		
		mPtArray->PushPt(tempPt);
		
		average += tempPt;
	}
	
	Point3DFloat tetCtr = average / 4.0;
	this->SetCtr(tetCtr);	// need this to make the Volume Calculations work right
	
	// Now tetPts has four random points
	// Make a set of Sides and push them onto me
	Side tempSide;
	for (short j=0; j<4; j++) {
		tempSide.pt1 = tetPts[j];
		tempSide.pt2 = tetPts[(j+1) % 4];
		tempSide.pt3 = tetPts[(j+2) % 4];
		AlignSide(tempSide);
		tempSide.MakeInVect(tetCtr);
		PushSide(tempSide);
	}

	// Discard the interior points
	for (short k=0; k < mPtArray->GetCount(); k++) {
		if (PointInBox((*mPtArray)[k]) && !PointOnBox((*mPtArray)[k])) {
			mPtArray->RemoveItem(k);
		}
	}
}

// ---------------------------------------------------------------------------
//		¥ FindFirstSide
// ---------------------------------------------------------------------------
/* This is part of the convex hull operation.  This method finds the first side
of the bounding box to start things off.  It does this by making a fake horizontal
side out of the point with the minimum Z coordinate, and then using this to find the 
first real side.  It then verifies the side, looking for any points in the dataset 
that are outside this Side. */
void
BoundingBox::FindFirstSide()
{
	NumPtArray coplanars;
	NumberedPt thisPt, fakePt1, fakePt2, newPt, betterPt;

	fakePt1.seq = -1;
	fakePt1.x = zMin.x;
	fakePt1.y = zMin.y + 1;
	fakePt1.z = zMin.z;

	fakePt2.seq = -2;
	fakePt2.x = zMin.x + 1;
	fakePt2.y = zMin.y;
	fakePt2.z = zMin.z;

	Side thisSide;
	thisSide.pt1 = zMin;
	thisSide.pt2 = fakePt1;
	thisSide.pt3 = fakePt2;
	
	AlignSide(thisSide);

	newPt = BendAroundSide(coplanars, thisSide.pt1, thisSide.pt2, thisSide.pt3);	// ignore coplanars for now; this side will not be a real side either

	thisSide.pt2 = newPt;	// newPt is our second good point in the side
	AlignSide(thisSide);

	newPt = BendAroundSide(coplanars, thisSide.pt1, thisSide.pt2, thisSide.pt3);
	thisSide.pt3 = newPt;	// newPt is our third good point in the side
	AlignSide(thisSide);

	while (!VerifySide(thisSide, &betterPt, &coplanars)) {
		// while there are points outside this Side:
		double dist1 = thisSide.pt1.Distance(betterPt);
		double dist2 = thisSide.pt2.Distance(betterPt);
		double dist3 = thisSide.pt3.Distance(betterPt);
		// replace the point that is closest to the better point
		if (myMin(dist1, dist2, dist3) == dist1)
			thisSide.pt1 = betterPt;
		else if (myMin(dist1, dist2, dist3) == dist2)
			thisSide.pt2 = betterPt;
		else
			thisSide.pt3 = betterPt;
		AlignSide(thisSide);
		FindCoplanars(coplanars, thisSide);
	}
	
	if (coplanars.GetCount() > 0) {
		// make the coplanar points into a set of sides
		SideifyCoplanars(thisSide, coplanars);
	} else {
		AlignSide(thisSide);
		thisSide.MakeInVect(mCtr);
		PushSide(thisSide);
	}
}

// ---------------------------------------------------------------------------
//		¥ SideifyCoplanars
// ---------------------------------------------------------------------------
/* In the event that we have a bunch of coplanar points on the boundary, we want
to have the most efficient coverage of this area without altering the data.
This way we accomplish this is to find all of the coplanar points, and draw a
2d bounding box around them (egads!).  Then, the most efficient coverage is to
make all of the triangles originate from one point on the boundary and connect
all pairs of consecutive points on the bounding box. The points pt1, pt2 and
pt3 are all in the plane in question...
// This could be improved to deal with collinear points...  Here's the algorithm idea:
1. You know that ioSide.pt1 & ioSide.pt2 are part of the next side, because they 
were part of the last side.
2. You find the point i whose vector from point 2 makes the biggest angle with
the 2->1 vector.  This is point 3, and it is on the bounding polygon for the
region.  If there are any collinear points, choose the one that's furthest from point 2.
3. You find the point i whose vector from point 3 makes the biggest angle with
the 3->2 vector.  This is point 4, and it is on the bounding polygon for the
region.  If there are any collinear points, choose the one that's furthest from point 3.
[ do 2&3 until you get back to point 1 ]
4.  You now know all the points on the 2-D bounding curve.  Toss out all the others.
5.  Make the sides like: 1-2-3, 1-3-4, 1-4-5, 1-5-6, etc.
*/
void
BoundingBox::SideifyCoplanars (Side &ioSide, NumPtArray &coplanars)
{
	short	i, bigAnglePt;
	double maxAngle, testAngle, testDist, bigAngleDist;
	bool backToPt1 = false;
	Point3DFloat  vectNtoNm1, vectNtoi, vectNto1, temp;
	NumPtArray planarArray;
	NumPtArray bb2d;	// 2-D bounding box
	
	if (GetCount() > 0) {
		// the first two points are always the good ones when we get here from BendAroundSide.  They
		// must be part of the 2-D Bounding Box.
		bb2d.PushPt(ioSide.pt1);
		bb2d.PushPt(ioSide.pt2);

		// We add all the possible other points to the coplanar array.  We don't add Points 1 & 2
		// of the Side, because those are fixed by the previous side, and cannot be removed from
		// consideration, and must be part of the 2-D bounding box
		planarArray.PushPt(ioSide.pt3);
		for (i = 0; i <= coplanars.GetCount() -1; i++) {
			planarArray.PushPt(coplanars[i]);
		}
	} else {
		/* When this function is called from FindFirstSide, there are no sides yet found, so we get here.
		In that case, ioSide.p1 and ioSide.pt2 may *not* in fact be at the edge of the 2d bounding box.
		So the first thing we'll do is add all the points to the planarArray from which we will select the
		points in the 2-D Bounding Box (those on the edges of the set). */
		planarArray.PushPt(ioSide.pt2);
		planarArray.PushPt(ioSide.pt1);
		planarArray.PushPt(ioSide.pt3);
		for (i = 0; i <= coplanars.GetCount() - 1; i++) {
			planarArray.PushPt(coplanars[i]);
		}
		
		// find an extreme point, by picking the furthest point from a randomly chosen one (ioSide.pt1).
		// This point must be along an edge.
		double curDist, farDist = 0;
		NumberedPt	thisPt, farPt;
		short	farPtNum = -1;
		for (i = 0; i <= planarArray.GetCount() - 1; i++) {
			thisPt = planarArray[i];
			curDist = ioSide.pt1.Distance(thisPt);
			if (curDist > farDist) {
				farDist = curDist;
				farPt = thisPt;
				farPtNum = i;
			}
		}
		bb2d.PushPt(farPt);	// add this far point to the 2-D bounding box
		planarArray.RemoveItem(farPtNum);	// remove the point from the planarArray
		
		/* next, we wish to find a point that makes the largest angle with some random point in the set,
			using our extreme point as the vertex.  Should there be more than one with the same angle
			(i.e., collinear), we want the one furthest from our extreme point.  We'll use ioSide.pt2 
			as our random point. */
		Point3DFloat vectRandToXtrm, vectNtoXtrm;
		NumberedPt	maxAnglePt;
		short	maxAnglePtNum;
		vectRandToXtrm = farPt - ioSide.pt2;
		maxAngle = 0;
		bigAngleDist = 0;
		
		// look through the planar array, and determine the angle (ioSide.pt2 - farPt - thisPt) ("testAngle")
		for (i = 0; i <= planarArray.GetCount() - 1; i++) {
			thisPt = planarArray[i];
			vectNtoXtrm = farPt - thisPt;
			testAngle = vectRandToXtrm.Angle(vectNtoXtrm);
			testDist = vectNtoXtrm.Magnitude();
			if ((testAngle > maxAngle) || ((testAngle == maxAngle) && (testDist > bigAngleDist))) {
				maxAngle = testAngle;
				maxAnglePt = thisPt;
				maxAnglePtNum = i;
				bigAngleDist = testDist;
			}
		}
		bb2d.PushPt(maxAnglePt);	// add this point to the 2-D Bounding Box
		planarArray.RemoveItem(maxAnglePtNum);	// remove it from the planarArray list
	}
	
	/* we now have in 2dbb, two points that must be on the edge of the true
		2-D Bounding Box.  This is our starting point.  We now essentially repeat the previous
		algorithm, looking for points that maximize the angle (previousPt - thisPt - testPt)
		and thereby make our way around the 2-D Bounding Box until we arrive again at point 1. */
	while (!backToPt1) {
		maxAngle = 0;
		bigAngleDist = 0;
		temp = bb2d[bb2d.GetCount()-2];	// this line cannot be combined with the next due to the way I return values
										// from the operator[] function
		vectNtoNm1 = temp - bb2d[bb2d.GetCount()-1];
		for (i = 0; i <= planarArray.GetCount() - 1; i++) {
			temp = planarArray[i];
			vectNtoi = temp - bb2d[bb2d.GetCount()-1];
			testAngle = vectNtoNm1.Angle(vectNtoi);
			temp = planarArray[i];
			testDist = bb2d[bb2d.GetCount()-1].Distance(temp);
			if ((testAngle == maxAngle && testDist > bigAngleDist) || (testAngle > maxAngle)) {
				maxAngle = testAngle;
				bigAnglePt = i;
				bigAngleDist = testDist;
			}
		}
		temp = bb2d[0];
		vectNto1 = temp - bb2d[bb2d.GetCount()-1];
		testAngle = vectNtoNm1.Angle(vectNto1);	// what is the angle (n-1 - n - pt1)?
		// if the angle (n-1 - n - n+1) is less than (n-1 - n - pt1), then the point n+1 is
		// inside the 2-D bounding box, and we should not add it to the bb2d array.  Instead
		// we should exit the loop, because we are done: we've found the 2D Bounding Box.
		if (testAngle > maxAngle) {
			backToPt1 = true;
		} else {
			bb2d.PushPt(planarArray[bigAnglePt]);
			planarArray.RemoveItem(bigAnglePt);
		}
	}
	
	// bb2d now holds the 2-D bounding box points in order, and we are done with planar array (it's
	// probably empty, anyhow.
	short secondToLast = bb2d.GetCount() - 2;
	for (i = 1; i <= secondToLast; i++) {  // this range for i is because each side is made up of
										   // points 0, i, and i+1
		Side newSide;
		newSide.pt1 = bb2d[0];
		newSide.pt2 = bb2d[i];
		newSide.pt3 = bb2d[i+1];
		AlignSide(newSide);
		newSide.MakeInVect(mCtr);
		newSide.edge1 = CheckSides(newSide.pt1, newSide.pt2);
		newSide.edge2 = CheckSides(newSide.pt2, newSide.pt3);
		newSide.edge3 = CheckSides(newSide.pt1, newSide.pt3);
		PushSide(newSide);
	}
	
}   // SideifyCoplanars procedure

// ---------------------------------------------------------------------------
//		¥ FindCoplanars
// ---------------------------------------------------------------------------
/* Finds all points coplanar with the side. */
void
BoundingBox::FindCoplanars (NumPtArray &coplanars, Side &inSide)
{
	Point3DFloat norm = inSide.AwayVect(mCtr);	// orthogonal vect to side; points out
	
	coplanars.Clear();
	for (short i = 0; i <= mPtArray->GetCount() - 1; i++) {
		if (!inSide.PointOnSide((*mPtArray)[i])) {
			double newTheta = norm.Angle((*mPtArray)[i] - inSide.pt1);
			if (newTheta - M_PI_2 <= kCoplanarThreshold) {	// angle is near 90¡, or less than
																// 90¡ (means point is outside
																// plane, so include it no matter
																// what - by finding that out here,
																// we know it must be close) 
				coplanars.PushPt((*mPtArray)[i]);
			}
		}
	}
}


// ---------------------------------------------------------------------------
//		¥ VerifySide
// ---------------------------------------------------------------------------
/* Checks to be sure that new side is not inside any points.  For each point, we
determine the angle between (a) the vector from a side vertex to the point and 
(b) the out-pointing normal to the side.  For points inside the Side, this angle will
be greater than 90 degrees.  For points outside, it will be less than 90 degrees. */
bool
BoundingBox::VerifySide (Side &inSide, NumberedPt* ioBetterPt, NumPtArray *coplanars)
{
	Point3DFloat away = inSide.AwayVect(mCtr);
	double angle, minAngle = M_PI;
	
	for (short i = 0; i <= mPtArray->GetCount() - 1; i++) {
		NumberedPt curPt = (*mPtArray)[i];
		if ((!inSide.PointOnSide(curPt)) &&	// point isn't part of side and...
			(coplanars == nil || !coplanars->PointInArray(curPt))) {	// point isn't in coplanars array, if provided
			Point3DFloat curVect = curPt - inSide.pt1;	// vector from pt1 to curPt
			angle = curVect.Angle(away);
			if ((angle < M_PI / 2.0) && (angle < minAngle)) {
				minAngle = angle;
				if (ioBetterPt != nil)
					*ioBetterPt = curPt;
			}
		}
	}
	if (minAngle < M_PI / 2.0)
		return false;
	return true;
}

// ---------------------------------------------------------------------------
//		¥ FindNextEdge
// ---------------------------------------------------------------------------
/* This routine looks through the list of sides and finds the next edge which
is unfinished (i.e. only has one surface attached to it).  Returns the number
of the side and edge which it selects.  The bool value returned is true if
it finds something, or false if it does not (i.e. everything is finished!) */
bool
BoundingBox::FindNextEdge (short &nextSide, short &nextEdge)
{
	Side curSide;
	short numSides = GetCount();
	
	nextEdge = 0;
	for (short i=0; (i <= numSides-1) && (nextEdge == 0); i++) {
		curSide = (*this)[i];
		if (curSide.edge1 == kUnfinished ) {
			nextEdge = 1;
			nextSide = i;
		} else if (curSide.edge2 == kUnfinished ) {
			nextEdge = 2;
			nextSide = i;
		} else if (curSide.edge3 == kUnfinished ) {
			nextEdge = 3;
			nextSide = i;
		}
	}
	return (nextEdge > 0);
}  // FindNextEdge procedure /%/


// ---------------------------------------------------------------------------
//		¥ FoldEdge
// ---------------------------------------------------------------------------
/* This routine takes one side and finds the next one adjoining one of its edges.
The edge dealt with is specified in the variable "nextEdge": 1 == between points
1 and 2, 2 == between points 2 and 3, and 3 == between points 3 and 1. */
void
BoundingBox::FoldEdge (short nextSide, short nextEdge)
{
	NumberedPt newPt;
	NumPtArray coplanars;
	Side	newSide, curSide;
	
	curSide = (*this)[nextSide];
	if (nextEdge == 1 ) {
		newPt = BendAroundSide(coplanars, curSide.pt1, curSide.pt2, curSide.pt3);
		newSide.pt1 =  curSide.pt2;
		newSide.pt2 =  curSide.pt1;
		curSide.edge1 = kFinished;
	}
	if (nextEdge == 2 ) {
		newPt = BendAroundSide(coplanars, curSide.pt2, curSide.pt3, curSide.pt1);
		newSide.pt1 =  curSide.pt3;
		newSide.pt2 =  curSide.pt2;
		curSide.edge2 = kFinished;
	}
	if (nextEdge == 3 ) {
		newPt = BendAroundSide(coplanars, curSide.pt3, curSide.pt1, curSide.pt2);
		newSide.pt1 =  curSide.pt1;
		newSide.pt2 =  curSide.pt3;
		curSide.edge3 = kFinished;
	}
	UpdateSide(nextSide, curSide);
	newSide.pt3 = newPt;
	bool goodSide = VerifySide(newSide, nil, nil);
	if (goodSide) {
		if (coplanars.GetCount() > 0 )
			SideifyCoplanars(newSide, coplanars);
		else {
			AlignSide(newSide);
			newSide.MakeInVect(mCtr);
			newSide.edge1 = CheckSides(newSide.pt1, newSide.pt2);
			newSide.edge2 = CheckSides(newSide.pt2, newSide.pt3);
			newSide.edge3 = CheckSides(newSide.pt3, newSide.pt1);
			PushSide(newSide);
		}
	}
}

// ---------------------------------------------------------------------------
//		¥ BendAroundSide
// ---------------------------------------------------------------------------
/* Bends around the edge of the plane defined by pt1, pt2 and pt3 at the
edge defined by pt1 and pt2. Finds the point that makes a side having the
maximum angle between the vectors that go from the common edge (pt1-pt2) to
the third point. The variable coplanars returns the number of additional points
coplanar with the plane defined by the point returned, pt1, and pt2.*/
NumberedPt &
BoundingBox::BendAroundSide (NumPtArray &coplanars, NumberedPt &inPt1,
							 NumberedPt &inPt2, NumberedPt &inPt3)
{
	Side	tempSide;
	short		i, goodPoint;
	Point3DFloat lastSideOutVect;
	double		maxTheta, newTheta;
	NumberedPt		tempPt;
	coplanars.Clear();
	
	tempSide.pt1 = inPt1;
	tempSide.pt2 = inPt2;
	tempSide.pt3 = inPt3;
	Side startSide(tempSide);	// copy construct
	
	lastSideOutVect = tempSide.Vect12to3();	// this is a unit vector in the plane of the side, orthogonal to the 
											// 1-2 edge, pointing towards 3.  We're looking for the point that
											// makes the maximum angle with this vector (a coplanar point would
											// make a 180¡ (pi) angle).

	maxTheta = -1000;	// initialize max to something small
	for (i = 0; i <= mPtArray->GetCount() - 1; i++) {
		if (!startSide.PointOnSide((*mPtArray)[i])) {	// if this point is not on the side already
			tempSide.pt3 = (*mPtArray)[i];
			if (tempSide.MinAngle() > kCoplanarThreshold) {	// if the point is not collinear with the first two points of the side
				newTheta = lastSideOutVect.Angle(tempSide.Vect12to3());	// newTheta is the angle the candidate makes with the vector pointing towards 3 from the 1-2 edge
				if (::fabs(newTheta - maxTheta) <= kCoplanarThreshold)	// if the difference between newTheta and maxTheta is small, we call the point coplanar
					coplanars.PushPt((*mPtArray)[i]);
				else if (newTheta > maxTheta) {		// if newTheta is bigger than maxTheta, then we have a new max to use, so:
					maxTheta = newTheta;		// record the new max angle
					goodPoint = i;			// record the new best point
					coplanars.Clear();		// clear the coplanars array
				}
			}	// else the point is collinear; ignore it
		}
	}
	tempSide.pt3 = (*mPtArray)[goodPoint];	// record the best point in the side
	NumberedPt betterPt;
	while (!VerifySide(tempSide, &betterPt, &coplanars)) {	// check to see if we somehow missed a point.  Should not happen.
		tempSide.pt3 = betterPt;
		FindCoplanars(coplanars, tempSide);
	}
	
	if (!VerifySide(tempSide, &betterPt, &coplanars)) {	// check to see if we still, somehow, in this crazy world we live in, missed a point
		throw("Side failed to verify in BendAroundSide. Bad News...Dave should probably leave coding to the professionals.");
	}
	
	if (maxTheta < 0 || maxTheta > M_PI)
		throw("Problem in BendAroundSide.");   // This should never happen.  There should probably be a warning dialog that something's amiss... /%/
	
	return (*mPtArray)[goodPoint];
	
}

// ---------------------------------------------------------------------------
//		¥ CheckSides
// ---------------------------------------------------------------------------
/* This routine is used for bookkeeping -- once a new side is found, it looks
through all of the extant sides and sees if the new side adjoins any -- if
so, the appropriate edges are marked as "FINISHED". */
short
BoundingBox::CheckSides (Point3DFloat inPt1, Point3DFloat inPt2)
{
	short result = kUnfinished;
	Side curSide;
	short numSides = GetCount();
	short i;
	
	for (i=0; i <= (numSides - 1) && result != kFinished; i++) {
		curSide = (*this)[i];
		if (((inPt1 == curSide.pt1) && (inPt2 == curSide.pt2)) ||
			((inPt1 == curSide.pt2) && (inPt2 == curSide.pt1)))	{
			curSide.edge1 = kFinished;
			result = kFinished;
		} else if (((inPt1 == curSide.pt2) && (inPt2 == curSide.pt3)) ||
				   ((inPt1 == curSide.pt3) && (inPt2 == curSide.pt2))) {
			curSide.edge2 = kFinished;
			result = kFinished;
		} else if (((inPt1 == curSide.pt3) && (inPt2 == curSide.pt1)) ||
				   ((inPt1 == curSide.pt1) && (inPt2 == curSide.pt3))) {
			curSide.edge3 = kFinished;
			result = kFinished;
		}
	}	// this increments i when it leaves, even though the condition was false, so
		// we need to:
	i--;
	if (result == kFinished) {
		UpdateSide(i, curSide);
	}
	return result;
}


#pragma mark Other Bounding Box operations

// ---------------------------------------------------------------------------
//		¥ PointInBox
// ---------------------------------------------------------------------------
//
bool
BoundingBox::PointInBox(Point3DFloat &inPt, double tolerance)
{
	if (mType == kSidesBox) {
		// if it's inside the inscribed box, which we hope is a cube, then return true
		if (mInscribedBox != nil) {
			if (mInscribedBox->RawPointInBox(inPt)) {	// this is a fast check, only has 
														// (at most) 8 sides, maybe just math
				return true;
			}
		}
		// if it's outside the exscribed box, which is a cube, then return false
		if (mExscribedBox != nil) {
			if (!mExscribedBox->RawPointInBox(inPt)) {	// this is a really fast check done by math
				return false;
			}
		}
	}
	return RawPointInBox(inPt, tolerance);
}


// ---------------------------------------------------------------------------
//		¥ BetterInscribedBox
// ---------------------------------------------------------------------------
void
BoundingBox::BetterInscribedBox()
{
	if (mPrefs->verbose) mCalc->log("Making a BetterInscribedBox.\n");

	// First, delete the old inscribed box made in DiscardInteriorPoints
	if (mInscribedBox != nil) {
		delete mInscribedBox;
		mInscribedBox = nil;
	}
	if (mType == kRPBox || mType == kCylBox) {
		mInscribedBox = mExscribedBox = nil;	// then the box we have is fast, and there's no need
												// for an inscribed or exscribed box
	} else {		// we are a Sides box.  Operations on a Sides box are slow, so let's create an inscribed
					// and exscribed rectangular prism to speed things up
		if (mPrefs->verbose) mCalc->log("\tThis box is a SidesBox.\n");
		
		NumberedPt			xMin, xMax, yMin, yMax, zMin, zMax;
		double					increment;
		increment = GetMinDimension() / 100.0;
		
		// Push each point out from the center in the six orthogonal directions, by 1% increments,
		// until it's just outside the box, then pull it back by one increment.  The octahedron
		// formed by these points is a decent inscribed box, having 8 sides, but we still have to
		// do PointInBox side by side.
		
		xMin = mCtr;
		xMax = mCtr;
		yMin = mCtr;
		yMax = mCtr;
		zMin = mCtr;
		zMax = mCtr;
		
		do {
			xMin.x -= increment;
		} while (PointInBox(xMin));
		xMin.x += increment;
		
		do {
			yMin.y -= increment;
		} while (PointInBox(yMin));
		yMin.y += increment;
		
		do {
			zMin.z -= increment;
		} while (PointInBox(zMin));
		zMin.z += increment;
		
		do {
			xMax.x += increment;
		} while (PointInBox(xMax));
		xMax.x -= increment;
		
		do {
			yMax.y += increment;
		} while (PointInBox(yMax));
		yMax.y -= increment;
		
		do {
			zMax.z += increment;
		} while (PointInBox(zMax));
		zMax.z -= increment;

		if (mPrefs->verbose) {
			mCalc->log("\tFound points for OctahedronBox:");
			char logMsg[kStdStringSize];
			sprintf (logMsg, "\t\t xMin = (%f, %f, %f)\n", xMin.x, xMin.y, xMin.z);
			mCalc->log(logMsg);
			sprintf (logMsg, "\t\t yMin = (%f, %f, %f)\n", yMin.x, yMin.y, yMin.z);
			mCalc->log(logMsg);
			sprintf (logMsg, "\t\t zMin = (%f, %f, %f)\n", zMin.x, zMin.y, zMin.z);
			mCalc->log(logMsg);
			sprintf (logMsg, "\t\t xMax = (%f, %f, %f)\n", xMax.x, xMax.y, xMax.z);
			mCalc->log(logMsg);
			sprintf (logMsg, "\t\t yMax = (%f, %f, %f)\n", yMax.x, yMax.y, yMax.z);
			mCalc->log(logMsg);
			sprintf (logMsg, "\t\t zMax = (%f, %f, %f)\n", zMax.x, zMax.y, zMax.z);
			mCalc->log(logMsg);
		}	
		
		SideSet *OctahedronBox;
		OctahedronBox = new SideSet;
		OctahedronBox->SetType(kSidesBox);
		OctahedronBox->SetCtr(mCtr);
		Side thisSide;
		thisSide.pt1 = zMax;
		thisSide.pt2 = xMax;
		thisSide.pt3 = yMin;
		OctahedronBox->AlignSide(thisSide);	// shouldn't need this, but...
		OctahedronBox->PushSide(thisSide);
		thisSide.pt1 = zMax;
		thisSide.pt2 = yMin;
		thisSide.pt3 = xMin;
		OctahedronBox->AlignSide(thisSide);	// shouldn't need this, but...
		OctahedronBox->PushSide(thisSide);
		thisSide.pt1 = zMax;
		thisSide.pt2 = xMin;
		thisSide.pt3 = yMax;
		OctahedronBox->AlignSide(thisSide);	// shouldn't need this, but...
		OctahedronBox->PushSide(thisSide);
		thisSide.pt1 = zMax;
		thisSide.pt2 = yMax;
		thisSide.pt3 = xMax;
		OctahedronBox->AlignSide(thisSide);	// shouldn't need this, but...
		OctahedronBox->PushSide(thisSide);
		thisSide.pt1 = zMin;
		thisSide.pt2 = xMax;
		thisSide.pt3 = yMax;
		OctahedronBox->AlignSide(thisSide);	// shouldn't need this, but...
		OctahedronBox->PushSide(thisSide);
		thisSide.pt1 = zMin;
		thisSide.pt2 = yMax;
		thisSide.pt3 = xMin;
		OctahedronBox->AlignSide(thisSide);	// shouldn't need this, but...
		OctahedronBox->PushSide(thisSide);
		thisSide.pt1 = zMin;
		thisSide.pt2 = xMin;
		thisSide.pt3 = yMin;
		OctahedronBox->AlignSide(thisSide);	// shouldn't need this, but...
		OctahedronBox->PushSide(thisSide);
		thisSide.pt1 = zMin;
		thisSide.pt2 = yMin;
		thisSide.pt3 = xMax;
		OctahedronBox->AlignSide(thisSide);	// shouldn't need this, but...
		OctahedronBox->PushSide(thisSide);
		OctahedronBox->MakeAllInVects();
		
		// We make the inscribed cube, the cube that just fits inside the box
		double nearDist = NearestSideDist(mCtr) / sqrt(3.0);
		//	we divide by sqrt(3) because we're setting a perpendicular length, but the
		//	nearest side could be as far off as that of the (1 1 1) face.
		// If the distance were to the (1 1 1) face, then dividing by sqrt(3) would
		//	turn it into the correct orthogonal distance.
		// Note: this would be better if we made it an RP box instead
		
		SideSet *CubeBox;
		CubeBox = new SideSet;
		CubeBox->SetCtr(mCtr);
		CubeBox->SetType(kCubeBox);
		CubeBox->SetSideLen(nearDist);
		
		double tempLen;
		// we expand the inscribed cube until one of it's corners is outside the BBox
		while (PrimitiveInBox(CubeBox)) {
			tempLen = CubeBox->GetSideLen();
			CubeBox->SetSideLen(tempLen+increment);
		}
		// ...and then shrink it by one increment
		tempLen = CubeBox->GetSideLen();
		CubeBox->SetSideLen(tempLen-increment);

		if (mPrefs->verbose) {
			mCalc->log("\tFound inscribed CubeBox:");
			char logMsg[kStdStringSize];
			sprintf (logMsg, "\t\t center = (%f, %f, %f)\n", mCtr.x, mCtr.y, mCtr.z);
			mCalc->log(logMsg);
			sprintf (logMsg, "\t\t sideLen = %f\n", tempLen);
			mCalc->log(logMsg);
		}	
		
		// We test the incsribed cube to see how much volume it has relative to the octahedron.
		// If the cube is much smaller (by 50%), then we don't use the cube.
		// We give the cube an advantage since it is so much faster to calculate.	
		double CubeVolume = pow(CubeBox->GetSideLen(), 3);
		if (OctahedronBox->Volume() < (CubeVolume * 1.5)) {
			if (mPrefs->verbose) mCalc->log("\t Chose CubeBox.\n");
			delete OctahedronBox;
			mInscribedBox = CubeBox;
		} else {
			if (mPrefs->verbose) mCalc->log("\t Chose OctahedronBox.\n");
			delete CubeBox;
			mInscribedBox = OctahedronBox;
		}
		
		// We make the exscribed cube, the cube that just fits around the whole box
		double farXlDist = 0;
		Point3DFloat farDistPt;
		double thisDist;
		Crystal *thisXl;
		for (short i = 0; i <= theXls->GetNumXls() - 1; i++) {
			thisXl = (Crystal *) theXls->GetItemPtr(i);
			thisDist = thisXl->ctr.Distance(mCtr);
			if (thisDist > farXlDist) {
				farXlDist = thisDist;
			}
		}
		
		// delete the earlier excribed cube, if it exists
		if (mExscribedBox) delete mExscribedBox;
		mExscribedBox = new SideSet;
		mExscribedBox->SetCtr(mCtr);
		mExscribedBox->SetType(kCubeBox);
		mExscribedBox->SetSideLen(farXlDist*2.0);
		// This is only the right length if farDistPt is in one of the principal directions
		
		// we shrink the exscribed cube until there's a point outside it...
		while (mExscribedBox->AllPointsInBox(theXls)) {
			tempLen = mExscribedBox->GetSideLen();
			mExscribedBox->SetSideLen(tempLen-increment);
		}
		// ...and then expand it by one increment
		tempLen = mExscribedBox->GetSideLen();
		mExscribedBox->SetSideLen(tempLen+increment);

		if (mPrefs->verbose) {
			mCalc->log("\tFound exscribed CubeBox:");
			char logMsg[kStdStringSize];
			sprintf (logMsg, "\t\t center = (%f, %f, %f)\n", mCtr.x, mCtr.y, mCtr.z);
			mCalc->log(logMsg);
			sprintf (logMsg, "\t\t sideLen = %f\n", tempLen);
			mCalc->log(logMsg);
		}	
	}
}

// ---------------------------------------------------------------------------
//		¥ PrimitiveInBox
// ---------------------------------------------------------------------------
/*	For the Cube and the Rectangular Prism, we just test each of the vertices to
see if they are inside the box.  For the Cylinder, this won't work, so instead
we test 200 points along the top and bottom circles to see if they are all in the
box. */
bool
BoundingBox::PrimitiveInBox(SideSet *inPrim)
{
	Point3DFloat thisPt;
	Point3DFloat theCtr = inPrim->GetCtr();
	double theSide;
	
	switch (inPrim->GetType()) {
		case kCubeBox:
			theSide = inPrim->GetSideLen();
			thisPt.x = theCtr.x - theSide / 2;
			thisPt.y = theCtr.y - theSide / 2;
			thisPt.z = theCtr.z - theSide / 2;
			if (!PointInBox(thisPt))	// try (0,0,0)
				return false;
				
				thisPt.y = theCtr.y + theSide / 2;
			if (!PointInBox(thisPt))	// try (0,1,0)
				return false;
				
				thisPt.x = theCtr.x + theSide / 2;
			if (!PointInBox(thisPt))	// try (1,1,0)
				return false;
				
				thisPt.y = theCtr.y - theSide / 2;
			if (!PointInBox(thisPt))	// try (1,0,0)
				return false;
				
				thisPt.z = theCtr.z + theSide / 2;
			if (!PointInBox(thisPt))	// try (1,0,1)
				return false;
				
				thisPt.x = theCtr.x - theSide / 2;
			if (!PointInBox(thisPt))	// try (0,0,1)
				return false;
				
				thisPt.y = theCtr.y + theSide / 2;
			if (!PointInBox(thisPt))	// try (0,1,1)
				return false;
				
				thisPt.x = theCtr.x + theSide / 2;
			if (!PointInBox(thisPt))	// try (1,1,1)
				return false;
				break;
		case kRPBox:
			thisPt.x = theCtr.x - inPrim->GetXLen() / 2;
			thisPt.y = theCtr.y - inPrim->GetYLen() / 2;
			thisPt.z = theCtr.z - inPrim->GetZLen() / 2;
			if (!PointInBox(thisPt))	// try (0,0,0)
				return false;
				
				thisPt.y = theCtr.y + inPrim->GetYLen() / 2;
			if (!PointInBox(thisPt))	// try (0,1,0)
				return false;
				
				thisPt.x = theCtr.x + inPrim->GetXLen() / 2;
			if (!PointInBox(thisPt))	// try (1,1,0)
				return false;
				
				thisPt.y = theCtr.y - inPrim->GetYLen() / 2;
			if (!PointInBox(thisPt))	// try (1,0,0)
				return false;
				
				thisPt.z = theCtr.z + inPrim->GetZLen() / 2;
			if (!PointInBox(thisPt))	// try (1,0,1)
				return false;
				
				thisPt.x = theCtr.x - inPrim->GetXLen() / 2;
			if (!PointInBox(thisPt))	// try (0,0,1)
				return false;
				
				thisPt.y = theCtr.y + inPrim->GetYLen() / 2;
			if (!PointInBox(thisPt))	// try (0,1,1)
				return false;
				
				thisPt.x = theCtr.x + inPrim->GetXLen() / 2;
			if (!PointInBox(thisPt))	// try (1,1,1)
				return false;
				break;
		case kCylBox:			
			// Find points on top and bottom circles of the cylinder.  We'll optimize this by calculating the offset
			//	from the circle center to each point in the positive 45 degree octant, and then use symmetry to find the other points
			for (short i = 1; i <= kNumOctantPoints; i++) {
				// theta is the angle in radians that corresponds to the i / numOctantPoints fraction of 45 degrees (pi/4)
				double theta = (M_PI / 4.0) * (double) i / (double) kNumOctantPoints;
				// xOffset and yOffset are the components of a vector from the center of the circle to the point on the 
				//	circle theta radians clockwise from North
				double xOffset = inPrim->GetRadius() * sin(theta);
				double yOffset = inPrim->GetRadius() * cos(theta);
				double zOffset = inPrim->GetHeight() / 2.0;
				
				// For each theta value, find 16 points that use the same X&Y offset values (some switched)				
				for (short xSign = -1; xSign <= 1; xSign += 2) {
					for (short ySign = -1; ySign <= 1; ySign += 2) {
						for (short zSign = -1; zSign <= 1; zSign += 2) {
							Point3DFloat offsetVector (xOffset * xSign, yOffset * ySign, zOffset * zSign);
							thisPt = theCtr + offsetVector;
							if (!PointInBox(thisPt)) return false;
							
							Point3DFloat offsetVectorSwitched (yOffset * ySign, xOffset * xSign, zOffset * zSign);
							thisPt = theCtr + offsetVectorSwitched;
							if (!PointInBox(thisPt)) return false;
						}
					}
				}
			}
			return true;
			break;
		case kSidesBox:
		default:
			throw("There was a problem in PrimitiveInBox");
			break;
	}
	return true;
}

// ---------------------------------------------------------------------------
//		¥ GetMinDimension
// ---------------------------------------------------------------------------
double
BoundingBox::GetMinDimension()
{
	double outMin;
	
	switch (mType) {
		case kCubeBox:
			outMin = GetSideLen();
			break;
		case kRPBox:
			outMin = myMin(GetXLen(), GetYLen(), GetZLen());
			break;
		case kCylBox:
			outMin = dmh_min(GetRadius()*2, GetHeight());
			break;
		case kSidesBox:
			// This is a quick hack to get a reasonably small dimension, but will not be the
			//	true minimum dimension.
			outMin = myMin(xMax.Distance(xMin), yMax.Distance(yMin), zMax.Distance(zMin));
			break;
		default:
			throw("Problem in GetMinDimension");
			break;
	}
	
	return outMin;
}

// ---------------------------------------------------------------------------
//		¥ RandPtInPrimitiveNotHole
// ---------------------------------------------------------------------------
void
BoundingBox::RandPtInPrimitiveNotHole(Point3DFloat &outPt, HoleSet *inHoles)
{
	if (inHoles) {
		do {
			outPt = RandPtInPrimitive();
		} while (inHoles->PointInHole(outPt));
	} else {
		outPt = RandPtInPrimitive();
	}
}



// ---------------------------------------------------------------------------
//		¥ Inflate
// ---------------------------------------------------------------------------
/*	This expands each vertex of the bounding box by %150 of the radius of the
	crystal at that point.  It's not the most efficient algorithm, since it will
	do each vertex 3 times. This process ensures that all the crystals will be
	inside the bounding box, but creates an artificially low crystal density */
void
BoundingBox::Inflate()
{
	short	i;
	Side	curSide;
	Crystal thisXl;
	
	for (i = 0; i <= GetCount() - 1; i++) {
		curSide = array[i];
		if (theXls->FindCrystalAt(curSide.pt1, &thisXl))
			curSide.pt1 += (((curSide.pt1 - mCtr).Unit()) * (thisXl.r * 1.5));
		if (theXls->FindCrystalAt(curSide.pt2, &thisXl))
			curSide.pt2 += (((curSide.pt2 - mCtr).Unit()) * (thisXl.r * 1.5));
		if (theXls->FindCrystalAt(curSide.pt3, &thisXl))
			curSide.pt3 += (((curSide.pt3 - mCtr).Unit()) * (thisXl.r * 1.5));
		UpdateSide(i, curSide);
	}
}

// ---------------------------------------------------------------------------------
//		¥ GetMCVolFracGranularity
// ---------------------------------------------------------------------------------
double
BoundingBox::GetMCVolFracGranularity(short mode)
{
	long numTries;
	switch (mode) {
		case kPrefValRough:
			numTries = mPrefs->MCReps / kRoughMCReductionFactor;
		break;
		case kPrefVal:
		default:
			numTries = mPrefs->MCReps;
		break;
	}
	return (1.0 / numTries);
}


// ---------------------------------------------------------------------------------
//		¥ GetVolumeFraction
// ---------------------------------------------------------------------------------
/* Since we can't do this accurately enough going crystal-by-crystal, due to possibly
	many multiply-intersecting crystals, we'll use a monte carlo method. */
double
BoundingBox::GetVolumeFraction(double inMaxRadius, long Tries, HoleSet *inHoles)
{
	long numInside1=0, numInside=0;
    char logMsg[kStdStringSize];
	Point3DFloat boxPt;
	Crystal *thisXl;
	std::vector<int> nearby;
	long numTries;
	long numNearby;
	switch (Tries) {
		case kPrefVal:
			numTries = mPrefs->MCReps;
		break;
		case kPrefValRough:
			numTries = mPrefs->MCReps / kRoughMCReductionFactor;
		break;
		default:
			numTries = Tries;
		break;
	}
	if (numTries >= 20)
		numTries = 20 * (numTries / 20);	// makes it divisible by 20;
	
	mCalc->log("Calculating Volume Fraction\n");
	mCalc->setupProgress("Determining volume fraction using monte carlo method.", nil, nil, "Reducing", -1, 1, numTries/20, 1, false);

//	this will give you the exact same "random" simulation every time
//	srand48(1234567);

	if (!theXls->VerifyList()) {
		throw("Unable to verify crystal list in GetVolumeFraction");
	}

	//  The whole deal with dividing by 20, is so we don't have to call progress for every single monte carlo
	//	point.  There will be many, so we only call it every 20th point.
	if (numTries >= 20) {
	// Randomize lots of points in box & keep track of what fraction are in any of the spheres
		for (long j=1; j <= numTries/20; j++) {
			mCalc->progress(j);
			if (mCalc->shouldStopCalculating()) throw(kUserCanceledErr);
			for (long innerJ = 1; innerJ <= 20; innerJ++) {
                sprintf (logMsg, "In BoundingBox::GetVolumeFraction. (j, innerJ)= (%li, %li)\n", j, innerJ);
                mCalc->log(logMsg);

				// boxPt is a random point in the bounding box (it uses the primitive to help do the calculation)
				RandPtInPrimitiveNotHole(boxPt, inHoles);
				
				// nearby is a vector of CrystalArray indices for crystals that are within maxRadius of the boxPt
				// (in the X-dimension). That is a fast operation, because we get nearby from the sorted list of
				// crystals; this keeps us from having to check every crystal in the array to see if the point
				// is inside the crystal.
				nearby = theXls->NearbyXls(boxPt, inMaxRadius);
				numNearby = nearby.size();
				for (short i = 0; i <= numNearby - 1; i++) {
					// we check each of the nearby crystals to see if the point is inside any of them
					thisXl = (Crystal *) theXls->GetItemPtr(nearby[i]);
					if (thisXl->PointInside(boxPt)) {
						numInside1++;
						break;	// break out of the for loop
					}
				}
			}
		}
	} else {
		mCalc->postError("The monte carlo repetitions should be a large number, at least 1000.  Your value was too small to give good results.",
						 "Bad Monte Caro Value", nil, -1, 5);
		for (long j=1; j <= numTries; j++) {
			mCalc->progress(j);
			if (mCalc->shouldStopCalculating()) throw(kUserCanceledErr);
			RandPtInPrimitiveNotHole(boxPt, inHoles);
			nearby = theXls->NearbyXls(boxPt, inMaxRadius);
			numNearby = nearby.size();
			for (short i = 0; i <= numNearby; i++) {
				thisXl = (Crystal *) theXls->GetItemPtr(nearby[i]);
				if (thisXl->PointInside(boxPt)) {
					numInside1++;
					break;
				}
			}
		}
	}
	numInside = numInside1;

	return ((double) numInside / (double) numTries);
}

// ---------------------------------------------------------------------------
//		¥ VolumeMinusGuard
// ---------------------------------------------------------------------------
/* Returns the volume of the bounding box.  Note that if there are overlapping
triangles on planes, the value will be too high */
double
BoundingBox::VolumeMinusGuard(HoleSet *holes, double guardWidth)
{
	double outVol;
	long numBoxPts = 0;
	long goodPts = 0;
	Point3DFloat tryPoint;
	long MCTries = mPrefs->MCReps;
	
	if (!holes) {
		switch (mType) {
			{ case kSidesBox:
				SideSet *mEC = GetExscribedBox();
				for (long i = 1; i <= MCTries; i++) {
					tryPoint = mEC->RandPtInPrimitive();
					if (RawPointInBox(tryPoint)) {
						numBoxPts++;
						if (NearestSideDist(tryPoint) > guardWidth) {
							goodPts++;
						}
					}
				}
				outVol = Volume()*goodPts/numBoxPts;
				break;
            }
			{ case kCubeBox:
				if (mSideLen - 2 * guardWidth > 0)
					outVol = pow((mSideLen - 2 * guardWidth), 3);
				else
					outVol = 0;
				break;
            }
			{ case kRPBox:
				if ((mSideLenX - 2 * guardWidth) < 0 ||
					(mSideLenY - 2 * guardWidth) < 0 ||
					(mSideLenZ - 2 * guardWidth) < 0)
					outVol = 0;
				else
					outVol = ((mSideLenX - 2 * guardWidth) * 
							  (mSideLenY - 2 * guardWidth) *
							  (mSideLenZ - 2 * guardWidth));
				break;
            }
			{ case kCylBox:
				if ((mHeight - 2 * guardWidth) < 0 ||
					(mRadius - guardWidth < 0))
					outVol = 0;
				else
					outVol = ((mHeight - 2 * guardWidth) * 
							  M_PI * sqr(mRadius - guardWidth));
				break;
            }
		}
	} else {	// else we have a HolesSet to deal with
		switch (mType) {
			{ case kSidesBox:
				SideSet *mEC = GetExscribedBox();
				for (long i = 1; i <= MCTries; i++) {
					tryPoint = mEC->RandPtInPrimitive();
					if (RawPointInBox(tryPoint)) {
						numBoxPts++;
						if ((NearestSideDist(tryPoint) > guardWidth) &&
							(holes->NearestHoleDist(tryPoint) > guardWidth)) {
							goodPts++;
						}
					}
				}
				outVol = Volume()*goodPts/numBoxPts;
				break; }
			{ case kCubeBox:
			case kRPBox:
			case kCylBox:
				for (long i = 1; i <= MCTries; i++) {
					tryPoint = RandPtInPrimitive();
					if (RawPointInBox(tryPoint)) {
						numBoxPts++;
						if ((NearestSideDist(tryPoint) > guardWidth) &&
							(holes->NearestHoleDist(tryPoint) > guardWidth)) {
							goodPts++;
						}
					}
				}
				outVol = Volume()*goodPts/numBoxPts;
				break; }
		}
	}
	return outVol;
}

// ---------------------------------------------------------------------------
//		¥ NearestSideDist
// ---------------------------------------------------------------------------
/* NearestSide -- The distance from a point to the nearest side of the bounding
box. From CRC Handbook, 25th edition, p.297 */
double
BoundingBox::NearestSideDist(Point3DFloat &inPt)
{
	double xDist, yDist, zDist;
	Point3DFloat pt1, pt2, pt3;
	Side thisSide;
	
	switch (mType) {
		case kCubeBox:
			xDist = (mSideLen / 2.0) - ::fabs(inPt.x - (mCtr.x + mOffset.x));
			yDist = (mSideLen / 2.0) - ::fabs(inPt.y - (mCtr.y + mOffset.y));
			zDist = (mSideLen / 2.0) - ::fabs(inPt.z - (mCtr.z + mOffset.z));
			return myMin(xDist, yDist, zDist);
			break;
		case kRPBox:
			xDist = (mSideLenX / 2.0) - ::fabs(inPt.x - (mCtr.x + mOffset.x));
			yDist = (mSideLenY / 2.0) - ::fabs(inPt.y - (mCtr.y + mOffset.y));
			zDist = (mSideLenZ / 2.0) - ::fabs(inPt.z - (mCtr.z + mOffset.z));
			return myMin(xDist, yDist, zDist);
			break;
		case kCylBox:
			double rDist;
			rDist = mRadius - sqrt(sqr(inPt.x - (mCtr.x + mOffset.x)) +
								   sqr(inPt.y - (mCtr.y + mOffset.y)));
			zDist = (mHeight / 2.0) - ::fabs(inPt.z - (mCtr.z + mOffset.z));
			return dmh_min(rDist, zDist);
			break;
		case kSidesBox:
			double	minDist, thisDist;
			double	a, b, c, d;		// coefficients of the plane equation
			short	numSides;
			numSides = GetCount();
			
			minDist = HUGE_VAL;
			for (short i  = 0 ; i  <= numSides - 1; i ++) {
				thisSide = (*this)[i];
				pt1 = thisSide.pt1;
				pt2 = thisSide.pt2;
				pt3 = thisSide.pt3;
				a = (pt2.y - pt1.y) * (pt3.z - pt1.z);
				a -= (pt2.z - pt1.z) * (pt3.y - pt1.y);
				b = (pt2.z - pt1.z) * (pt3.x - pt1.x);
				b -= (pt2.x - pt1.x) * (pt3.z - pt1.z);
				c = (pt2.x - pt1.x) * (pt3.y - pt1.y);
				c -= (pt2.y - pt1.y) * (pt3.x - pt1.x);
				d = -(pt1.x * a) - (pt1.y * b) - (pt1.z * c);
				thisDist = ::fabs(a * inPt.x + b * inPt.y + c * inPt.z + d) / sqrt(a * a + b * b + c * c);
				if (thisDist < minDist )
					minDist = thisDist;
			}   // for i
				return minDist;
			break;
		default:
			throw("Problem in NearestSideDist");
			break;
	}
	throw("NearestSideDist: Shouldn't Be Here!");
	return positiveInfinity; 	// should never get here!
}   // NearestSideDist function

// ---------------------------------------------------------------------------
//		¥ SetOffset
// ---------------------------------------------------------------------------
void
BoundingBox::SetOffset(Point3DFloat &inPt)
{
	mInscribedBox->SetOffset(inPt);
	mExscribedBox->SetOffset(inPt);
	SideSet::SetOffset(inPt);
}


// ---------------------------------------------------------------------------
//		¥ AdjustToBounds
// ---------------------------------------------------------------------------
/*	After adjusting the bounds of the Crystal Array, we come here.  In AdjustToBounds,
we adjust the boundary values of the SideSet (this), and we remove any crystals
that are outside the new bounds */
void
BoundingBox::AdjustToBounds()
{
	if ((mType == kCubeBox) || (mType == kRPBox)) {
		Point3DFloat Lower, Upper;
		Lower = theXls->LowerBound();
		Upper = theXls->UpperBound();
		xMax = yMax = zMax = Upper;
		xMin = yMin = zMin = Lower;
		mCtr = (Lower + Upper) / 2.0;
		mType = kRPBox;
		mSideLenX = Upper.x - Lower.x;
		mSideLenY = Upper.y - Lower.y;
		mSideLenZ = Upper.z - Lower.z;
	} else {	// then primitive is cylinder
		mHeight = theXls->GetHeight();
		mRadius = theXls->GetRadius();
		mCtr = theXls->GetCtr();
		mType = kCylBox;
		xMax = yMax = zMax = xMin = yMin = zMin = mCtr;
		xMax.x = mCtr.x + mRadius;
		xMin.x = mCtr.x - mRadius;
		yMax.y = mCtr.y + mRadius;
		yMin.y = mCtr.y - mRadius;
		zMax.z = mCtr.z + 0.5 * mHeight;
		zMin.z = mCtr.z - 0.5 * mHeight;
	}
	for (short i=0; i <= theXls->GetNumXls()-1; i++) {
		Crystal *thisXl = (Crystal *) theXls->GetItemPtr(i);
		if (!PointInBox(thisXl->ctr)) {
			theXls->RemoveItem(i, false);
			i--;
		}
	}
	theXls->RebuildList();
}

// ---------------------------------------------------------------------------
//		¥ XlTouchesBox
// ---------------------------------------------------------------------------
//
bool
BoundingBox::XlTouchesBox(Crystal &inXl)
{
	double d = NearestSideDist(inXl.ctr);
	return (d <= inXl.r);
}




// ---------------------------------------------------------------------------
//		¥ Theta
// ---------------------------------------------------------------------------
/* This function, stolen from Sedgewick, 1983, takes two points and calculates
a number between 0 and 360 which has the same ordering properties as the true
angle between the line defined by the points and horizontal. This is used
instead of an arctan function because it's faster (fewer special cases, no
												   calls to trig functions). */
double
BoundingBox::Theta (double x1, double y1, double x2, double y2)
{
	double	dx, dy, ax, ay, t;
	
	dx = x2 - x1;
	ax = ::fabs(dx);
	dy = y2 - y1;
	ay = ::fabs(dy);
	if ((dx == 0.0) && (dy == 0.0))
		t = 0.0;
	else
		t = dy / (ax + ay);
	if (dx < 0.0)
		t = 2.0 - t;
	else if (dy < 0)
		t = 4.0 + t;
	return (t * 90.0);
}  // Theta function



// ---------------------------------------------------------------------------
//		¥ SurfaceArea
// ---------------------------------------------------------------------------
double
BoundingBox::SurfaceArea()
{
	double	area = 0.0;
	switch (mType) {
		case kCubeBox:
			area = GetSideLen() * GetSideLen() * 6.0;
			break;
		case kRPBox:
			area = 2.0 * (GetXLen() * GetYLen() + GetXLen() * GetZLen() + GetZLen() * GetYLen());
			break;
		case kCylBox:
			area = 2.0 * M_PI * sqr(GetRadius()) + GetHeight() * 2.0 * M_PI * GetRadius();
			break;
		case kSidesBox:
			short	i;
			Side	curSide;
			
			for (i = 0; i <= GetCount() - 1; i++) {
				curSide = (*this)[i];
				area += curSide.Area();
			}
				break;
	}
	return area;
}


// ---------------------------------------------------------------------------
//		¥ MakeRandomSimulation
// ---------------------------------------------------------------------------
/*	This is one of the key components of the envelope creation.  It forms the
	basis for Calculator::MakeGoodRandomSimulation.  It uses an array of crystal
	radii, which are required to replicate the CrystalArray of the actual data
	set.  We have the option of additionally trying to replicate the volume
	fraction of the data set as well.  */
void
BoundingBox::MakeRandomSimulation(
								  std::vector<double> &inRadiiList, // this is an array of radii found in the actual crystal data set, sorted by decreasing size
								  bool inMatchingCTDataSet, // should we match the VF of the actual data set?
								  double inVolFraction, // the VF of the actual crystal data set
								  HoleSet* inHoles, // holes to avoid
								  double betafactorfactor	// a factor involved in a sort of "diffusion-controlled" simulation.
															// Only used when making D.C. envelopes.  Defaults to zero.
								  )
{
//	If you uncomment these, you'll get the exact same "random" simulation every time
//	srand48(1234567);

	Crystal	thisXl;
	long XlNum;
	bool goodSimulation = false;
	nuclProbType *nuclProb = nil;
	short obs1=0, obs2=0;
	bool goodPlacing;
	int numXls = inRadiiList.size();
	
	theXls->Clear();
	if (mPrefs->matchPDF) {
		nuclProb = theXls->GetNuclProb();
	}
	
	mCalc->setupProgress("Making a randomized crystal array", nil, nil, nil, -1, 0, numXls, 1, false);
	
	for (short SimTry = 1; SimTry <= kNumSimMakerRestarts && !goodSimulation; SimTry++) {
		
		//	We try a small number of times (kNumSimMakerRestarts).  The restart will be
		//	triggered when we are unable to successfully place one of the crystals after
		//	trying kNumTriesToPlaceXl times (a large number).
		bool unableToPlace = false;
		theXls->Clear();
		
		for (XlNum = 0; (XlNum <= numXls-1) && !unableToPlace; XlNum++) {
			
			// for each crystal we try to place
			mCalc->progress(XlNum);
			if (mCalc->shouldStopCalculating()) throw(kUserCanceledErr);
			thisXl.r = inRadiiList[XlNum];
			goodPlacing = false;
			
			for (long numTries = 0; (numTries < kNumTriesToPlaceXl) && !goodPlacing; numTries++) {
				// keep trying to place the crystal until we've exceeded the desired number of tries or 
				// we've made a good placing (e.g., not intersecting).  Note that "intersecting" here does
				// not mean a strict intersection criterion but rather intersecting more than the 
				// interface-controlled growth rate law would permit (or if chosen, the Daniel-esque
				// diffusion-controlled growth rate law), together with the observability
				// criteria.
				
				thisXl.ctr = RandPtInPrimitive(nuclProb);	// note that nuclProb may be nil
				short intersects = theXls->CrystalIntersects(thisXl, inMatchingCTDataSet, inVolFraction, betafactorfactor);
				if (intersects == kObs1Criterion)
					obs1++;
				if (intersects == kObs2Criterion)
					obs2++;

				if (intersects == kNoIntersection) {
					if (inHoles == nil) {
						goodPlacing = true;
					} else {	// inHoles != nil
						goodPlacing = !(inHoles->PointInHole(thisXl.ctr));
						// if there is not intersection, but we have a holes array, then the placing
						// is only good if it is not within a hole
					}
				}
			} // end for numTries
			
			if (goodPlacing) {
				theXls->PushXl(thisXl, false);
			} else {
				// we have tried kNumTriesToPlaceXl times to place crystal XlNum and failed every time
				// therefore, we must give up on this simulation and start over.  Perhaps we were just
				// unlucky in the early placings somehow.  This will bug out of the for(XlNum) loop
				// and dump us into another round of the for(SimTry) loop.
				unableToPlace = true;
			}
			
		} // end for XlNum
		
		if (!unableToPlace) {
			// if we got here (past the end of the XlNum loop), then we have either gone through all
			// the crystals and placed them successfully, or we have raised the unableToPlace flag.
			// if the former, then we should not go through another SimTry, so we note that we now
			// have achieved a good simulation:
			goodSimulation = true;
		}
	} // end for SimTry

	theXls->RebuildList();

	char logMsg[kStdStringSize];
	sprintf(logMsg, "Failed to place\t%d\tcrystals due to criterion 1, and\t%d\tcrystals due to criterion 2.\n", obs1, obs2);
	mCalc->log(logMsg);

	if (!goodSimulation) {
		if (!mPrefs->makeDCEnv) {	// for DC Envelopes, this error msg doesn't apply, because we expect to have
									// to adjust betafactorfactor iteratively
			mCalc->postError("Couldn't make IC Simulation for Envelope.  Do not quit - Will try again with finer grain.",
							 "Bad Simulation", nil, -1, 3);
		}
		throw SimulationErr();
	}
}

void
BoundingBox::ExportBoxData() {
	CFStringRef inFileCFPath = (theXls->GetFile())->getPath();
	char inFileCString[kStdStringSize];
	CFStringGetCString (inFileCFPath, inFileCString, (CFIndex) kStdStringSize, kCFStringEncodingUTF8);
	std::string inFilePath (inFileCString);
	int lastPeriod = inFilePath.find('.');
	std::string boxFilePath = inFilePath.erase(lastPeriod);	// erase everything after the last period
	boxFilePath += "_bbox.txt";
	
	stringFile boxFile(true, boxFilePath.c_str());
	
	char tempLine[kStdStringSize];
	sprintf(tempLine, "Bounding Box Export for:\t%s", inFilePath.c_str());
	boxFile.putOneLine(tempLine);
	
	Point3DFloat tempCtr = CalcCtr();
	sprintf(tempLine, "Current Center:\t%f\t%f\t%f", tempCtr.x, tempCtr.y, tempCtr.z );
	boxFile.putOneLine(tempLine);
	
	switch (mType) {
		case kCubeBox:
			
			break;
		case kRPBox:
			
			break;
		case kCylBox:
			
			break;
		case kSidesBox:
			short	i;
			Side	curSide;
			
			for (i = 0; i <= GetCount() - 1; i++) {
				curSide = (*this)[i];
				sprintf(tempLine, "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f",
						curSide.pt1.x, curSide.pt1.y, curSide.pt1.z,
						curSide.pt2.x, curSide.pt2.y, curSide.pt2.z,
						curSide.pt3.x, curSide.pt3.y, curSide.pt3.z,
						curSide.inVect.x, curSide.inVect.y, curSide.inVect.z);
				boxFile.putOneLine(tempLine);
			}
			break;
	}
}


