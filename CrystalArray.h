// =================================================================================
//	CrystalArray.h 					
// =================================================================================

#pragma once

#import "Crystal.h"
#import "ReduceConstants.h"
#import "ReduceTypes.h"
#import <string>
#import <vector>

#define kNoIntersection 0
#define kObs1Criterion 1
#define kObs2Criterion 2
#define kIC_or_DCCriterion 3

class stringFile;
class XListElement;
class Calculator;

class CrystalArray {

	public:	// Exception Classes
		class ArrayIOErr {};
		class	BadOrderErr {
			public:
				short expectedNum;
				short foundNum; };
		class MemErr{};
		class PSRProblem {};
		class MergeIOErr {
			public:
				std::string briefDesc;
				std::string longDesc; };

		public:
							CrystalArray(Calculator *inCalc, long inNumExpected = 600);
							CrystalArray(Calculator *inCalc, CrystalArray &inXls);
							CrystalArray(Calculator *inCalc, CrystalArray *inXls);
							~CrystalArray();
		void				Copy(CrystalArray *inXls);
		void				InitXlArray(CrystalArray *inXls);
		
		// ARRAY METHODS
		Crystal				PopXl(bool inKeepListCorrect);
		void				PushXl(Crystal &inXl, bool inKeepListCorrect);
		Crystal *			operator[](int inSub);
		Crystal *			GetItemPtr(int i);
		inline int			GetNumXls() {return array.size();};
		void				Clear();
		void				UpdateXl(int index, Crystal *inXl,bool inKeepListCorrect);
		void				RemoveItem(int inAtIndex, bool inKeepListCorrect);
		
		// FILE METHODS
		inline void			SetFile(stringFile *inFile) {mFile = inFile;};
		stringFile *		GetFile() {return mFile;};
		void				ReadMergeHeader();
		void				ReadMergeFile();
		inline std::string	&GetIntComment() {return mIntComment;};
		inline fileTypes	GetFileType() {return mFileType;};
		
		// BOUNDS METHODS (Some input files have explicit bounds)
		inline boundsTypes	GetBounds() {return mBounds;};
		inline Point3DFloat &
							LowerBound() {return mLower;};
		inline Point3DFloat &
							UpperBound() {return mUpper;};
		inline void			SetBounds(boundsTypes inBounds) {mBounds = inBounds;};
		inline void			SetLower (Point3DFloat inLower) {mLower = inLower;};
		inline void			SetUpper (Point3DFloat inUpper) {mUpper = inUpper;};
		inline Point3DFloat	&	GetCtr () {return mCtr;};
		inline float		GetRadius () {return mRadius;};
		inline float		GetHeight () {return mHeight;};
		inline void			SetCtr (Point3DFloat inCtr) {mCtr = inCtr;};
		inline void			SetRadius (float inRadius) {mRadius = inRadius;};
		inline void			SetHeight (float inHeight) {mHeight = inHeight;};

		// TEXTURAL MEASURING METHODS
		short				CrystalIntersects(Crystal &inXl, bool inMatchingCTDataSet, float inVolFraction=0, float betafactorfactor=0);
		bool				FindCrystalAt(Point3DFloat inPt, Crystal *thisXl);
		Point3DFloat &		Average();
		double				GetMeanRadius();
		inline short		GetNegRadii() {return mNegRadii;};
		void				FilterForObservability();
		nuclProbType*		GetNuclProb();
		void				ChangeRadii(double *outRadiiList, float inDelta);
		bool				AdjustLocations();
		float				CorrectedCrystalVolume(Crystal *inXl);
		void				CorrectForImpingement();
		void				RemoveIllegalOverlaps();

	protected:
		stringFile *		mFile;
		fileTypes			mFileType;
		std::string			mIntComment;
		int					mNumCrystals;	// used in reading Merge Files.  Holds alleged number of crystals as stated in header.
		float				mTotalVolume;
		boundsTypes			mBounds;
		Point3DFloat		mLower, mUpper;
		Point3DFloat		mCtr;
		float				mRadius, mHeight;
		int					mNegRadii;
		nuclProbType		*mNuclProb;
		PrefStruct *		mPrefs;
		Calculator *		mCalc;

	private:
		std::vector<Crystal> array;

// ---------------------------------------------------------------------------
/* xList Data and Methods

	The xList is intended to speed up certain search operations by sorting the crystals
	by the x-coordinate of their centers.  Most methods keep this list sorted as crystals
	are added or removed from the main Crystal array.  Some methods return std::vector<int>s, which
	are just STL vectors of ints, each of which is an array index in the main Crystal array.
	Formerly, the xList was a standard C-array, but here it has been implemented as an
	STL vector.  If performance is slow, switching back to a C-array might be warranted.
*/
	public:
		void				RebuildList();
		std::vector<int>	NearbyXls(Point3DFloat inPt, float inDistance);
		bool				VerifyList();

protected:
//		void				AddToList(XListElement *inList, float inVal, long inIndex, long *ioListSizeAlloc);
//		bool				RemoveFromList(XListElement *inList, float inVal, long inIndex);
		void				SortInternalList();
		short				NearbyCount(float inVal, float inDistance,
										int &outLowerIndex, int &outUpperIndex);

		
	protected:	// member variables
		bool				mListIsSorted;
		std::vector<XListElement>
							mList;
};

class XListElement {
	public:
		inline bool operator== (const XListElement &inEl) {return (inEl.index==index);};
		inline bool operator< (const XListElement &inEl) {return (location < inEl.location);};
		inline bool operator> (const XListElement &inEl) {return (location > inEl.location);};
		friend class CrystalArray;
//		friend bool CompareListElement (XListElement inA, XListElement inB);
		static inline bool CompareListElement (XListElement inA, XListElement inB) { return inA.location < inB.location; };	
	protected:
		int		index;
		float	location;
};

// I cannot get operator< to work with the STL sort, so here's a function to do the job instead
//inline bool CompareListElement (XListElement inA, XListElement inB) { return inA.location < inB.location; };	

