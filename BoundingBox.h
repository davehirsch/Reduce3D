// ===========================================================================
//	BoundingBox.h
// ===========================================================================
#pragma once

#include "SideSet.h"
#include "CrystalArray.h"
#include "NumPtArray.h"
#include "ReduceConstants.h"
#include "ReduceTypes.h"
#include "Point3DFloat.h"
#include "Matrix4d.h"
#import "CoreFoundation/CoreFoundation.h"
#import <vector>

enum VFCodes {kPrefVal = -1, kPrefValRough = -2};

// ---------------------------------------------------------------------------
//		¥ BoundingBox
// ---------------------------------------------------------------------------
class stringFile;
class Calculator;
class HoleSet;

class	BoundingBox : public SideSet {
	public:
		class SimulationErr{};
	
	friend class Calculator;

	public:
						BoundingBox(Calculator *inCalc);
						BoundingBox(Calculator *inCalc, stringFile *inFile);
						BoundingBox(BoundingBox &inBBox);
		virtual			~BoundingBox();
		void			Copy(BoundingBox &inBBox);
		bool			PointInBox(Point3DFloat &inPt);
		bool			XlTouchesBox(Crystal &inXl);
		void			FindConvexHull();
		void			PrepForHullOrPrimitive();
		bool			CheckedAgainstBounds();
		float			SurfaceArea();
		float			GetMaxDimension();
		float			GetMinDimension();
		float			VolumeMinusGuard(HoleSet *holes, float guardWidth);
		float			GetXMax(){return xMax.x;};
		float			GetYMax(){return yMax.y;};
		float			GetZMax(){return zMax.z;};
		CrystalArray *	GetXls() {return theXls;};
		double			NearestSideDist(Point3DFloat &inPt);
		float			GetPercentSphereInside(Point3DFloat inCtr, float inRadius, short inNumSpherePts);
		void			Inflate();
		void			SetOffset(Point3DFloat &inPt);
		void			saveShavedIntegrateFile(short inShaveIteration);
		void			writeIntegrateFile(stringFile *saveFile, const char *firstLine, const char *commentLine);
		void			AdjustToBounds();
		bool			PrimitiveInBox(SideSet *inPrim);
		
	protected:
		void			FindBoundPoints();
		void			FindCenter();
		void			DiscardInteriorPoints();
		void			FindFirstSide();
		NumberedPt		MinThetaPt(NumberedPt &inPt1, NumberedPt &inPt2,
								NumberedPt &inPt3, Matrix4d &inMatrix,
							    NumPtArray coplanars);
		float			Theta(float x1, float y1, float x2, float y2);
		NumberedPt		&BendAroundSide (NumPtArray &coplanars, NumberedPt &pt1,
								NumberedPt &pt2, NumberedPt &pt3);
		void			FindCoplanars (NumPtArray &coplanars, Side &inSide);
		bool			VerifySide (Side &inSide, NumberedPt* betterPt = nil, NumPtArray *coplanars = nil);
		void			SideifyCoplanars (Side &ioSide, NumPtArray &coplanars);
		short			CheckSides (Point3DFloat inPt1, Point3DFloat inPt2);
		bool			FindNextEdge (short &nextSide, short &nextEdge);
		void			FoldEdge (short nextSide, short nextEdge);
		void			BetterInscribedBox();
		void			FindBetterCenter();
		void			ForceVolumeFraction(float inVolFraction);
		void			FixVolumeFraction(float inVolFraction);
		float			GetVolumeFraction(float inMaxRadius, long Tries = kPrefVal, HoleSet *inHoles = nil);
		void			RandPtInPrimitiveNotHole(Point3DFloat &outPt, HoleSet *inHoles = nil);
		float			GetMCVolFracGranularity(short mode);
		void			MakeRandomSimulation(std::vector<double> &inRadiiList, //double *inRadiiList,
												bool inMatchingCTDataSet, 
												float inVolFraction, 
												HoleSet* inHoles=nil, 
												float betafactorfactor=0);

	protected:
		CrystalArray *		theXls;
		NumPtArray *		mPtArray;
		NumberedPt			xMin, xMax, yMin, yMax, zMin, zMax;
		PrefStruct *		mPrefs;
		Calculator *		mCalc;

};
