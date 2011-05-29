// ===========================================================================
//	BoundingBox.h
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
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
		bool			PointInBox(Point3DFloat &inPt, double tolerance=0);
		bool			XlTouchesBox(Crystal &inXl);
		void			FindConvexHull();
		void			PrepForHullOrPrimitive();
		bool			CheckedAgainstBounds();
		double			SurfaceArea();
		double			GetMaxDimension();
		double			GetMinDimension();
		double			VolumeMinusGuard(HoleSet *holes, double guardWidth);
		double			GetXMax(){return xMax.x;};
		double			GetYMax(){return yMax.y;};
		double			GetZMax(){return zMax.z;};
		CrystalArray *	GetXls() {return theXls;};
		double			NearestSideDist(Point3DFloat &inPt);
		double			GetPercentSphereInside(Point3DFloat inCtr, double inRadius, short inNumSpherePts);
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
		void			FindInitialTetrahedron();
		bool			UnfacetedPointsExist();
		void			FindNextVertexAndMakeFacets();
		void			FindFirstSide();
		NumberedPt		MinThetaPt(NumberedPt &inPt1, NumberedPt &inPt2,
								NumberedPt &inPt3, Matrix4d &inMatrix,
							    NumPtArray coplanars);
		double			Theta(double x1, double y1, double x2, double y2);
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
		void			ForceVolumeFraction(double inVolFraction);
		void			FixVolumeFraction(double inVolFraction);
		double			GetVolumeFraction(double inMaxRadius, long Tries = kPrefVal, HoleSet *inHoles = nil);
		void			RandPtInPrimitiveNotHole(Point3DFloat &outPt, HoleSet *inHoles = nil);
		double			GetMCVolFracGranularity(short mode);
		void			MakeRandomSimulation(std::vector<double> &inRadiiList, //double *inRadiiList,
												bool inMatchingCTDataSet, 
												double inVolFraction, 
												HoleSet* inHoles=nil, 
												double betafactorfactor=0);
		void			ExportBoxData();

	protected:
		CrystalArray *		theXls;
		NumPtArray *		mPtArray;
		NumberedPt			xMin, xMax, yMin, yMax, zMin, zMax;
		PrefStruct *		mPrefs;
		Calculator *		mCalc;

};
