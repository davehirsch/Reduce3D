// ===========================================================================
//	SideSet.h
//  Reduce3D
//
//  Created by David Hirsch on 10/1/97.
//  Copyright 2011 David Hirsch.
//  Distributed under the terms of the GNU General Public License v3
//	See file "COPYING for more info.
// ===========================================================================
#pragma once


#import "Side.h"
#import "ReduceConstants.h"
#import "ReduceTypes.h"
#import "MathStuff.h"
#import <vector>
#import "CoreFoundation/CoreFoundation.h"

class CrystalArray;
class stringFile;

// ---------------------------------------------------------------------------
//		¥ SideSet
// ---------------------------------------------------------------------------

class	SideSet{
	public:
		class		ArrayIOErr{};

							SideSet();
							SideSet(SideSet &inSideSet);
							~SideSet();
		void				Copy(SideSet &inSideSet);
		Side				PopSide();
		void				RemoveSide(int inAtIndex);
		void				PushSide(Side &inSide);
		void				Clear();
		Side				&operator[](short inSub);
		Side *				GetItemPtr(short inSub);
		void				UpdateSide(short index, Side &inSide);
		bool				RawPointInBox(Point3DFloat &inPt, double tolerance=0);
		inline void			SetType(short inType) {mType = inType;};
		inline void			SetCtr(Point3DFloat inPt) {mCtr = inPt;};
		Point3DFloat&		CalcCtr();
		inline void			SetSideLen(double inLen) {mSideLen = inLen;};
		double				Volume();
		inline double		GetSideLen() {return mSideLen;};
		Point3DFloat&		GetSideLenPt();
		inline Point3DFloat& GetCtr() {return mCtr;};
		inline SideSet*		GetExscribedBox() {return (mType == kRPBox ? this : mExscribedBox);};
		inline SideSet*		GetInscribedBox() {return (mType == kRPBox ? this : mInscribedBox);};
		bool				AllPointsInBox(CrystalArray *theXls);
		double				ToroidalDistance(Point3DFloat &inPt1, Point3DFloat &inPt2);
		inline void			SetOffset(Point3DFloat &inPt) {mOffset = inPt;};
		inline Point3DFloat	GetOffset() {return mOffset;};
//		Point3DFloat		&RandPtInPrimitive();
//		void				RandPtInPrimitive(Point3DFloat &ioPt);
		Point3DFloat &		RandPtInPrimitive(nuclProbType *nuclProb = nil);
		double				Calc1Probability(nuclProb1D &nuclProb, double inLoc);
		short				NumPointsInBox(CrystalArray *theXls);
		void				SetDimensions(double inXLen, double inYLen, double inZLen);
		void				SetDimensions(double inRadius, double inHeight);
		void				SetDimensions(Point3DFloat inPt);
		inline short		GetType(){return mType;};
		inline double		GetRadius(){return mRadius;};
		inline double		GetHeight(){return mHeight;};
		inline double		GetXLen(){return mSideLenX;};
		inline double		GetYLen(){return mSideLenY;};
		inline double		GetZLen(){return mSideLenZ;};
		void				AlignSide(Side &ioSide);
		bool				PointOnBox(Point3DFloat &inPt);
		void				MakeAllInVects();
		double				AspectRatio();
		inline short		GetCount() {return array.size();};

	protected:
		double		TetrahedronVolume (Point3DFloat &pt1, Point3DFloat &pt2,
								Point3DFloat &pt3, Point3DFloat &pt4);

	protected:
//		double			mDiscardThreshold;
		short			mType;
		NumberedPt		mCtr;
		double			mSideLen, mSideLenX, mSideLenY, mSideLenZ, mHeight, mRadius;
		SideSet			*mInscribedBox;
		SideSet			*mExscribedBox;
		Point3DFloat	mOffset;
		std::vector<Side> array;


};
